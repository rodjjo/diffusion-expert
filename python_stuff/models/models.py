import gc
import os

from transformers import CLIPVisionModelWithProjection, CLIPVisionConfig
from diffusers import (
    StableDiffusionPipeline,
    StableDiffusionImageVariationPipeline
)

from models.paths import (BASE_DIR, MODELS_DIR)
from models.loader import load_stable_diffusion_model

MODEL_EXTENSIONS = set(['.ckpt', '.safetensors'])
CURRENT_MODEL_PARAMS = {}
CURRENT_PIPELINE = {}
CURRENT_VAR_PIPELINE = {}

# if the model does not load see: https://github.com/d8ahazard/sd_dreambooth_extension/discussions/794

def load_model(model_path: str):
    global CURRENT_MODEL_PARAMS
    if CURRENT_MODEL_PARAMS.get('path', '') != model_path:
        CURRENT_MODEL_PARAMS = {
            'path': model_path,
            'params': load_stable_diffusion_model(model_path)
        }
    gc.collect()


def create_pipeline(model_path: str):
    global CURRENT_PIPELINE
    global CURRENT_VAR_PIPELINE
    load_model(model_path)
    if CURRENT_PIPELINE.get("model_path") != model_path:
        CURRENT_PIPELINE = {
            'model_path': model_path,
            'pipeline': StableDiffusionPipeline(**CURRENT_MODEL_PARAMS['params'])
        }
    CURRENT_VAR_PIPELINE = {}
    gc.collect()
    return CURRENT_PIPELINE['pipeline']


def create_var_pipeline(model_path: str):
    global CURRENT_PIPELINE
    global CURRENT_VAR_PIPELINE
    load_model(model_path)
    if CURRENT_VAR_PIPELINE.get("model_path") != model_path:
        mparams = CURRENT_MODEL_PARAMS['params']
        image_model = CLIPVisionModelWithProjection(CLIPVisionConfig())
        params = {
            'vae': mparams['vae'],
            'image_encoder': image_model,
            'unet': mparams['unet'],
            'scheduler':  mparams['scheduler'],
            'safety_checker': mparams['safety_checker'],
            'feature_extractor':  mparams['feature_extractor'],
        }
        CURRENT_VAR_PIPELINE = {
            'model_path': model_path,
            'pipeline': StableDiffusionImageVariationPipeline(**params)
        }
    CURRENT_PIPELINE = {}
    gc.collect()
    return CURRENT_VAR_PIPELINE['pipeline']


def is_model(path: str):
    n = path.lower()
    for e in MODEL_EXTENSIONS:
        if n.endswith(e):
            return True
    return False


def list_models(directory: str):
    files = [n for n in os.listdir(directory) if is_model(n)]
    path = lambda n: os.path.join(directory, n)
    return [{
        "path": path(n),
        "name": n,
        "size": os.stat(path(n)).st_size,
        "hash": "not-computed",
    } for n in files]