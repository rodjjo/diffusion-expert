# from safetensors import safe_open
import os
import sys

import torch
from models.models import create_pipeline

def txt2img(params: dict):
    def do_it():
        prompt = params['prompt']
        negative = params['negative']
        if len(negative or '') < 2:
            negative = None
        seed = params['seed']
        model = params["model"]
        print("loading model")
        pipeline = create_pipeline(model)
        pipeline.to("cuda")
        generator = None if seed == -1  else torch.Generator(device="cuda").manual_seed(seed)
        print("Generating Image")
        result = pipeline(prompt, guidance_scale=7.5, height=512, width=512, negative_prompt=negative, num_inference_steps=100, generator=generator).images[0]
        print("Completed")
        return {
                'data': result.tobytes(),
                'width': result.width,
                'height': result.height,
                'mode': result.mode,
            }
    data = do_it()        
    return data

    