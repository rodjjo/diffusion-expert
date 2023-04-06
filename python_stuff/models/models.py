from diffusers import (
    StableDiffusionPipeline,
)

from models.paths import (BASE_DIR, MODELS_DIR)
from models.loader import load_stable_diffusion_model
import gc

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


