# from safetensors import safe_open
import os
import sys

import torch
from torchvision import transforms
from PIL import Image
from models.models import create_pipeline, create_var_pipeline

def report(message):
    print(f"Text2image - {message}")

def txt2img(params: dict):
    def do_it():
        prompt = params['prompt']
        negative = params['negative']
        if len(negative or '') < 2:
            negative = None
        seed = params['seed']
        model = params["model"]
        cfg = params["cfg"]
        steps = params["steps"]
        width = params["width"]
        height = params["height"]

        variation_enabled = params.get('var_ref') is not None
        report("creating the pipeline")

        if variation_enabled:
            pipeline = create_var_pipeline(model) 
            seed += params['variation']
            data = params['var_ref']
            im = Image.frombytes(data['mode'], (data['width'], data['height']), data['data'])
            tform = transforms.Compose([
                transforms.ToTensor(),
                transforms.Resize(
                    (224, 224),
                    interpolation=transforms.InterpolationMode.BICUBIC,
                    antialias=False,
                    ),
                transforms.Normalize(
                [0.48145466, 0.4578275, 0.40821073],
                [0.26862954, 0.26130258, 0.27577711]),
            ])
            inp = tform(im).to('cuda').unsqueeze(0)
        else:
            pipeline = create_pipeline(model) 
            pipeline.to("cuda")
            inp = None

        generator = None if seed == -1  else torch.Generator(device="cuda").manual_seed(seed)

        if variation_enabled:
            report("generating the image variation")
            result = pipeline(inp, guidance_scale=cfg, generator=generator)[0]
        else:
            report("generating the image")
            result = pipeline(
                prompt, 
                guidance_scale=cfg, 
                height=height, 
                width=width, 
                negative_prompt=negative, 
                num_inference_steps=steps,
                generator=generator
            ).images[0]
        report("image generated")
        return {
                'data': result.tobytes(),
                'width': result.width,
                'height': result.height,
                'mode': result.mode,
            }
    data = do_it()        
    return data

    