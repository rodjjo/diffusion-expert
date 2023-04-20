import gc
import os
from diffusers import (
        StableDiffusionPipeline, 
        StableDiffusionControlNetPipeline, 
        StableDiffusionImg2ImgPipeline,
        StableDiffusionInpaintPipeline,
        ControlNetModel
    )
import torch
from models.paths import CACHE_DIR
from models.loader import load_stable_diffusion_model, set_pipeline_settings

MODEL_EXTENSIONS = set(['.ckpt', '.safetensors'])
CURRENT_MODEL_PARAMS = {}
CURRENT_PIPELINE = {}
CURRENT_VAR_PIPELINE = {}

# if the model does not load see: https://github.com/d8ahazard/sd_dreambooth_extension/discussions/794

def load_model(model_path: str):
    global CURRENT_MODEL_PARAMS
    if CURRENT_MODEL_PARAMS.get('path', '') != model_path:
        CURRENT_MODEL_PARAMS = {}
        gc.collect()
        params, in_painting = load_stable_diffusion_model(model_path)
        CURRENT_MODEL_PARAMS = {
            'path': model_path,
            'params': params,
            'in_painting': in_painting
        }
    gc.collect()


def create_pipeline(mode: str, model_path: str, controlnets = None):
    global CURRENT_PIPELINE
    global CURRENT_VAR_PIPELINE
    load_model(model_path)
    controlnet_modes = sorted([f["mode"] for f in (controlnets or [])])
    if CURRENT_PIPELINE.get("model_path") != model_path or \
            CURRENT_PIPELINE.get("contronet") != controlnet_modes or \
            mode != CURRENT_PIPELINE.get("mode"):
        CURRENT_PIPELINE = {}
        gc.collect()
        controlnets = controlnets or [] if mode == 'txt2img' else []
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
                model_repos[c['mode']], torch_dtype=torch.float16, cache_dir=CACHE_DIR
            ))

        if len(control_model) == 1:
            control_model = control_model[0]

        if have_controlnet:
            params = {
                **CURRENT_MODEL_PARAMS['params'],
                'controlnet': control_model,
            }
            pipe = StableDiffusionControlNetPipeline(**params)
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
    CURRENT_VAR_PIPELINE = {}
    gc.collect()
    return CURRENT_PIPELINE['pipeline']


def is_model(path: str):
    n = path.lower()
    for e in MODEL_EXTENSIONS:
        if n.endswith(e):
            return True
    return False

def current_model_is_in_painting():
    return CURRENT_MODEL_PARAMS.get('in_painting', False) is True

def list_models(directory: str):
    files = [n for n in os.listdir(directory) if is_model(n)]
    path = lambda n: os.path.join(directory, n)
    return [{
        "path": path(n),
        "name": n,
        "size": os.stat(path(n)).st_size,
        "hash": "not-computed",
    } for n in files]


def set_user_settings(config: dict):
    print("setting stable diffusion configurations")
    set_pipeline_settings(config)
