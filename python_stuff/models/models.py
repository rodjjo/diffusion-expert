import gc
import os
from contextlib import contextmanager
from diffusers import (
        StableDiffusionPipeline, 
        StableDiffusionControlNetPipeline, 
        StableDiffusionImg2ImgPipeline,
        StableDiffusionInpaintPipeline,
        ControlNetModel
    )

import safetensors

import torch
from models.paths import CACHE_DIR, MODELS_DIR, EMBEDDING_DIR, LORA_DIR
from utils.settings import get_setting
from utils.downloader import download_file
from models.loader import load_stable_diffusion_model
from external.img2img_controlnet import StableDiffusionControlNetImg2ImgPipeline
from external.img2img_inpaint_controlnet import StableDiffusionControlNetInpaintImg2ImgPipeline


CURRENT_MODEL_PARAMS = {}
CURRENT_PIPELINE = {}

# if the model does not load see: https://github.com/d8ahazard/sd_dreambooth_extension/discussions/794

def load_model(model_path: str):
    global CURRENT_MODEL_PARAMS
    global CURRENT_PIPELINE
    if CURRENT_MODEL_PARAMS.get('path', '') != model_path:
        CURRENT_MODEL_PARAMS = {}
        CURRENT_PIPELINE = {}
        gc.collect()
        params, in_painting = load_stable_diffusion_model(model_path)
        CURRENT_MODEL_PARAMS = {
            'path': model_path,
            'params': params,
            'in_painting': in_painting
        }
    gc.collect()

usefp16 = {
    True: torch.float16,
    False: torch.float32
}

def get_textual_inversion_paths():
    files = os.listdir(EMBEDDING_DIR)
    result = []
    for f in files:
        lp = f.lower()
        path = os.path.join(EMBEDDING_DIR, f) 
        if lp.endswith('.bin'): # rename .pt to bin before loading it
            result.append((False, path))
        elif lp.endswith('.safetensors'):
            result.append((True, path))
    return result

def get_lora_paths():
    files = os.listdir(LORA_DIR)
    result = []
    for f in files:
        lp = f.lower()
        path = os.path.join(LORA_DIR, f) 
        if lp.endswith('.ckpt') or lp.endswith('.safetensors'): # rename .pt to bin before loading it
            result.append(path)
    return result


def create_pipeline(mode: str, model_path: str, controlnets = None):
    global CURRENT_PIPELINE
    load_model(model_path)
    controlnet_modes = sorted([f["mode"] for f in (controlnets or [])])
    if CURRENT_PIPELINE.get("model_path") != model_path or \
            CURRENT_PIPELINE.get("contronet") != controlnet_modes or \
            mode != CURRENT_PIPELINE.get("mode"):
        CURRENT_PIPELINE = {}
        gc.collect()
        controlnets = controlnets or [] if mode in ('txt2img', 'img2img', 'inpaint2img') else []
        control_model = []
        have_controlnet = False
        model_repos = {
                'canny': 'lllyasviel/sd-controlnet-canny',
                'pose': 'lllyasviel/sd-controlnet-openpose',
                'scribble': 'lllyasviel/sd-controlnet-scribble',
                'deepth': 'lllyasviel/sd-controlnet-depth',
        }
        for c in controlnets:
            have_controlnet = True
            if not model_repos.get(c['mode']):
                print("No controlnet for ", c['mode'])
                continue
            print("Controlnet: ", c['mode'])
            control_model.append(ControlNetModel.from_pretrained(
                model_repos[c['mode']], torch_dtype=usefp16[get_setting('use_float16', True)], cache_dir=CACHE_DIR
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
            pipe = StableDiffusionImg2ImgPipeline(**CURRENT_MODEL_PARAMS['params'])
        elif mode == 'inpaint2img':
            pipe = StableDiffusionInpaintPipeline(**CURRENT_MODEL_PARAMS['params'])
        else:
            pipe = StableDiffusionPipeline(**CURRENT_MODEL_PARAMS['params'])
        # pipe.enable_model_cpu_offload()
        pipe.enable_attention_slicing(1)
        pipe.enable_xformers_memory_efficient_attention()
        CURRENT_PIPELINE = {
            'mode': mode,
            'model_path': model_path,
            'pipeline': pipe,
            'contronet': controlnet_modes
        }
        for tip in get_textual_inversion_paths():
            pipe.load_textual_inversion(tip[1], use_safetensors=tip[0], local_files_only=True)
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


def download_sd_model(url, filename):
    download_file(url, MODELS_DIR, filename)


def get_sd_model_urls():
    return [{
        'display_name': 'stable diffusion 1.5',
        'description': 'The original stable diffusion model',
        'format': 'fp32',
        'filename': 'v1-5-pruned.safetensors',
        'url': 'https://huggingface.co/runwayml/stable-diffusion-v1-5/resolve/main/v1-5-pruned.safetensors'
    }]


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
    files = get_lora_paths()
    for f in files:
        name = os.path.basename(f)
        if '.' in name:
            name = name.split('.', maxsplit=1)[0]
        result.append({
            'name': name,
            'kind': 'lora',
            'filename': os.path.basename(f),
            'path': f
        })
    return result
