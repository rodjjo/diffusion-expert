import gc
import os
from PIL import Image
from utils.images import pil_as_dict

from contextlib import contextmanager
from diffusers import (
        StableDiffusionPipeline, 
        StableDiffusionControlNetPipeline, 
        StableDiffusionImg2ImgPipeline,
        StableDiffusionInpaintPipeline,
        AutoPipelineForText2Image,
        AutoPipelineForImage2Image,
        AutoPipelineForInpainting,
        ControlNetModel
    )

from diffusers.models.attention_processor import AttnProcessor2_0

import safetensors

import torch
from models.paths import CACHE_DIR, MODELS_DIR, EMBEDDING_DIR, LORA_DIR
from utils.settings import get_setting, settings_version
from utils.downloader import download_file
from models.loader import load_stable_diffusion_model, get_textual_inversion_paths, get_lora_paths
from external.img2img_controlnet import StableDiffusionControlNetImg2ImgPipeline
from external.img2img_inpaint_controlnet import StableDiffusionControlNetInpaintImg2ImgPipeline


CURRENT_MODEL_PARAMS = {}
CURRENT_PIPELINE = {}

# if the model does not load see: https://github.com/d8ahazard/sd_dreambooth_extension/discussions/794

def load_model(model_path: str, lora_list: list, reload_model: bool, for_inpaiting: bool = False, use_lcm = False):
    global CURRENT_MODEL_PARAMS
    global CURRENT_PIPELINE
    lora_list.sort()
    if CURRENT_MODEL_PARAMS.get('path', '') != model_path or \
            lora_list != CURRENT_MODEL_PARAMS.get('lora_list', []) or \
            reload_model or \
            settings_version() != CURRENT_MODEL_PARAMS.get('settings_version'):
        CURRENT_MODEL_PARAMS = {}
        CURRENT_PIPELINE = {}
        gc.collect()
        params, in_painting, xl_model, tiny_vae = load_stable_diffusion_model(model_path, lora_list=lora_list, for_inpainting=for_inpaiting, use_lcm=use_lcm)
        CURRENT_MODEL_PARAMS = {
            'settings_version': settings_version(),
            'path': model_path,
            'lora_list': lora_list,
            'params': params,
            'in_painting': in_painting,
            'xl_model':  xl_model,
            'tiny_vae': tiny_vae
        }
    gc.collect()

usefp16 = {
    True: torch.float16,
    False: torch.float32
}


def create_pipeline(mode: str, model_path: str, controlnets = None, lora_list=[], reload_model=False):
    current_mode = mode
    if mode.startswith('lcm_'):
        use_lcm = True
        mode = mode.split('lcm_', maxsplit=1)[1]
    else:
        use_lcm = False    
    global CURRENT_PIPELINE
    reload_model = reload_model or current_mode != CURRENT_PIPELINE.get("mode")
    load_model(model_path, lora_list, reload_model, mode == 'inpaint2img', use_lcm=use_lcm)
    controlnet_modes = sorted([f["mode"] for f in (controlnets or [])])
    if CURRENT_PIPELINE.get("model_path") != model_path or \
            CURRENT_PIPELINE.get("contronet") != controlnet_modes or \
            current_mode != CURRENT_PIPELINE.get("mode") or \
            reload_model or \
            settings_version() != CURRENT_PIPELINE.get('settings_version'):
        CURRENT_PIPELINE = {}
        gc.collect()
        controlnets = controlnets or [] if mode in ('txt2img', 'img2img', 'inpaint2img') and not current_model_is_xl_model() else []
        control_model = []
        have_controlnet = False
        model_repos = {
                'canny': 'lllyasviel/sd-controlnet-canny',
                'pose': 'lllyasviel/sd-controlnet-openpose',
                'scribble': 'lllyasviel/sd-controlnet-scribble',
                'deepth': 'lllyasviel/sd-controlnet-depth',
                'segmentation': 'lllyasviel/sd-controlnet-seg',
                'lineart': 'lllyasviel/control_v11p_sd15s2_lineart_anime',
                'mangaline': 'lllyasviel/control_v11p_sd15s2_lineart_anime',
        }
        for c in controlnets:
            have_controlnet = True
            if not model_repos.get(c['mode']):
                print("No controlnet for ", c['mode'])
                continue
            print("Controlnet: ", c['mode'])
            if c['mode'] == 'segmentation':
                mode_str = f"models--lllyasviel--sd-controlnet-seg"
            elif c['mode'] == 'lineart':
                mode_str = f"models--lllyasviel--control_v11p_sd15s2_lineart_anime"
            else:
                mode_str = f"models--lllyasviel--sd-controlnet-{c['mode']}"
            local_files_only = os.path.exists(os.path.join(CACHE_DIR, mode_str, 'snapshots'))
            control_model.append(ControlNetModel.from_pretrained(
                model_repos[c['mode']], torch_dtype=usefp16[get_setting('use_float16', True)], cache_dir=CACHE_DIR, local_files_only=local_files_only
            ))

        if len(control_model) == 1:
            control_model = control_model[0]

        if have_controlnet:
            params = {
                **CURRENT_MODEL_PARAMS['params'],
                'controlnet': control_model,
            }

            if mode == 'txt2img':
                pipe = StableDiffusionControlNetPipeline(**params)
            elif mode == 'inpaint2img':
                pipe = StableDiffusionControlNetInpaintImg2ImgPipeline(**params)
            else:
                pipe = StableDiffusionControlNetImg2ImgPipeline(**params)

        elif mode == 'img2img':
            if current_model_is_xl_model():
                pipe = AutoPipelineForImage2Image.from_pipe(
                    CURRENT_MODEL_PARAMS['params']['unet']
                )
            else:
                pipe = StableDiffusionImg2ImgPipeline(**CURRENT_MODEL_PARAMS['params'])
        elif mode == 'inpaint2img':
            if current_model_is_xl_model():
                pipe = AutoPipelineForInpainting.from_pipe(
                    CURRENT_MODEL_PARAMS['params']['unet']
                )
            else:
                pipe = StableDiffusionInpaintPipeline(**CURRENT_MODEL_PARAMS['params'])
        else:
            if current_model_is_xl_model():
                pipe =  AutoPipelineForText2Image.from_pipe(
                    CURRENT_MODEL_PARAMS['params']['unet']
                )
                pipe.enable_model_cpu_offload()
            else:
                pipe = StableDiffusionPipeline(**CURRENT_MODEL_PARAMS['params'])
        # pipe.enable_model_cpu_offload()
        if CURRENT_MODEL_PARAMS['tiny_vae']:
            pipe.vae = CURRENT_MODEL_PARAMS['tiny_vae']
        pipe.to('cuda')
        pipe.enable_attention_slicing()
        pipe.enable_xformers_memory_efficient_attention()
        pipe.unet.set_attn_processor(AttnProcessor2_0())

        CURRENT_PIPELINE = {
            'settings_version': settings_version(),
            'mode': current_mode,
            'model_path': model_path,
            'pipeline': pipe,
            'contronet': controlnet_modes
        }
    gc.collect()
    return CURRENT_PIPELINE['pipeline']


@contextmanager
def models_memory_checker():
    global CURRENT_MODEL_PARAMS
    global CURRENT_PIPELINE
    device_name = get_setting('device', 'cuda')
    params = CURRENT_MODEL_PARAMS.get('params', {})
    vae = params.get('vae')
    unet = params.get('unet')
    safety_checker = params.get('safety_checker')
    pipeline = CURRENT_PIPELINE.get('pipeline')

    should_release_memory = False

    if device_name != 'cpu':
        should_release_memory = torch.cuda.memory_usage(device=device_name) > 70
    else:
        should_release_memory = torch.cuda.memory_usage(device='cpu') > 65

    try:
        if should_release_memory:
            if device_name == 'cpu':
                CURRENT_MODEL_PARAMS = {}
                CURRENT_PIPELINE = {}
            else:
                if vae:
                    vae.to('cpu')
                if unet:
                    unet.to('cpu')
                if safety_checker:
                    safety_checker.to('cpu')
                if pipeline:
                    pipeline.to('cpu')
                torch.cuda.empty_cache()
        gc.collect()
        yield
    finally:
        if should_release_memory and device_name != 'cpu':
            torch.cuda.empty_cache()
            if vae:
                vae.to(device_name)
            if unet:
                unet.to(device_name)
            if safety_checker:
                safety_checker.to(device_name)
            if pipeline:
                pipeline.to(device_name)
            gc.collect()


def current_model_is_in_painting():
    return CURRENT_MODEL_PARAMS.get('in_painting', False) is True

def current_model_is_xl_model():
    return CURRENT_MODEL_PARAMS.get('xl_model', False) is True

def download_sd_model(url, filename):
    download_file(url, MODELS_DIR, filename)


def get_sd_model_urls():
    return [{
        'display_name': 'stable diffusion 1.5',
        'description': 'The original stable diffusion model',
        'format': 'fp32',
        'filename': 'v1-5-pruned.safetensors',
        'url': 'https://huggingface.co/runwayml/stable-diffusion-v1-5/resolve/main/v1-5-pruned.safetensors'
    },
    {
        'display_name': 'stable diffusion 1.5 inpainting',
        'description': 'The original stable diffusion model inpainting',
        'format': 'fp32',
        'filename': 'v1-5-pruned.safetensors',
        'url': 'https://huggingface.co/runwayml/stable-diffusion-inpainting/resolve/main/sd-v1-5-inpainting.ckpt'
    },
    ]


def get_embeddings():
    files = get_textual_inversion_paths()
    result = []
    for f in files:
        if f[0]:
            data = safetensors.torch.load_file(f[1], device="cpu")
        else:
            data = torch.load(f[1], map_location="cpu")
        if 'string_to_param' in data:
            if 'name' in data:
                result.append({
                    'name': data['name'],
                    'kind': 'textual_inv',
                    'filename': os.path.basename(f[1]),
                    'path': f[1]
                })
            del data
        elif type(data) == dict and type(next(iter(data.values()))) == torch.Tensor:
            if len(data.keys()) != 1:
                continue
            emb = next(iter(data.values()))
            if len(emb.shape) == 1:
                emb = emb.unsqueeze(0)
            result.append({
                'name': next(iter(data.keys())),
                'kind': 'textual_inv',
                'filename': os.path.basename(f[1]),
                'path': f[1]
            })
            
    files = get_lora_paths()
    for f in files:
        name = os.path.basename(f)
        if '.' in name:
            name = name.rsplit('.', maxsplit=1)[0]
        result.append({
            'name': name,
            'kind': 'lora',
            'filename': os.path.basename(f),
            'path': f
        })
    for r in result:
        picture_path = f'{r["path"]}.jpg'
        if os.path.exists(picture_path):
            with Image.open(picture_path) as im:
                r['image'] = pil_as_dict(im)
    result.sort(key=lambda x: x['name'].lower())
    return result
