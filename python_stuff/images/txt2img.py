# from safetensors import safe_open
from diffusers import DiffusionPipeline

import gc 
import os
import sys


CACHE_DIR = os.path.join(os.path.dirname(sys.executable), '..', 'models', 'stable-diffusion')

def txt2img(params: dict):
    def do_it():
        prompt = params['prompt']
        negative = params['negative']
        model = params['model']
        print("Loading model")
        pipeline = DiffusionPipeline.from_pretrained('runwayml/stable-diffusion-v1-5', cache_dir=CACHE_DIR)
        pipeline.to("cuda")
        result = pipeline(prompt).images[0]
        return {
                'data': result.tobytes(),
                'width': result.width,
                'height': result.height,
                'mode': result.mode,
            }
    data = do_it()        
    gc.collect()
    return data

    