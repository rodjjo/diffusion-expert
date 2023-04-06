import gc
import os

from diffusers import (
    StableDiffusionPipeline,
)

from models.paths import (BASE_DIR, MODELS_DIR)
from models.loader import load_stable_diffusion_model

MODEL_EXTENSIONS = set(['.ckpt', '.safetensors'])
CURRENT_MODEL_PATH = None
CURRENT_PIPELINE = None

# if the model does not load see: https://github.com/d8ahazard/sd_dreambooth_extension/discussions/794

def create_pipeline(model_path: str):
    global CURRENT_PIPELINE
    global CURRENT_MODEL_PATH
    if CURRENT_MODEL_PATH == model_path:
        return CURRENT_PIPELINE
    del CURRENT_PIPELINE
    gc.collect()
    params = load_stable_diffusion_model(model_path)
    CURRENT_PIPELINE = StableDiffusionPipeline(**params)
    CURRENT_MODEL_PATH = model_path
    return CURRENT_PIPELINE


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