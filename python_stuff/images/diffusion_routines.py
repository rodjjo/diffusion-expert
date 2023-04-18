import gc

import torch
from models.models import create_pipeline
from images.latents import create_latents_noise, latents_to_pil
from exceptions.exceptions import CancelException
from PIL import Image
import PIL.ImageOps

from dexpert import progress, progress_canceled, progress_title


def report(message):
    progress_title(f'[Text To Image] - {message}')

def image_from_dict(data: dict):
    img = Image.frombytes(data['mode'], (data['width'], data['height']), data['data'])
    if img.mode.lower() == 'rgba':
        result = Image.new('RGB', (data['width'], data['height']),  (255, 255, 255))
        result.paste(img, (0, 0), img) 
        return result
    return img


def invert_image_from_dict(data: dict):
    return PIL.ImageOps.invert(image_from_dict(data))


@torch.no_grad()
def _run_pipeline(pipeline_type, params):
    device = "cuda"
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
    input_image = params.get("image")
    input_mask = params.get("mask")

    controlnets = params.get("controlnets", [])

    if width % 8 != 0:
        width += 8 - width % 8

    if height % 8 != 0:
        height += 8 - height % 8
    variation_enabled = params.get('var_stren', 0) > 0
    var_stren = params.get("var_stren", 0)
    subseed = params['variation'] if variation_enabled else None
    
    shape = (4, height // 8, width // 8 )
    latents_noise = create_latents_noise(shape, seed, subseed, var_stren)
    
    generator = None if seed == -1  else torch.Generator(device="cuda").manual_seed(seed)
    report("generating the variation" if variation_enabled else "generating the image")

    report("creating the pipeline")

    if pipeline_type == 'img2img' and input_mask is not None:
        pipeline_type = 'inpaint2img'

    pipeline = create_pipeline(pipeline_type, model, controlnets=controlnets) 

    def progress_preview(step, timestep, latents):
        progress(step, steps, latents_to_pil(step, pipeline.vae, latents))
        if progress_canceled():
            raise CancelException()
    
    additional_args = {}
    if pipeline_type == 'txt2img':
        additional_args = {
            'width': width, 
            'height': height,
            'latents': latents_noise,
        }
        if len(controlnets):
            images = []
            conds = []
            for c in controlnets:
                if c['strength'] < 0:
                    c['strength'] = 0
                if c['strength'] > 2.0:
                    c['strength'] = 2.0
                if c['mode'] == 'pose':
                    images.append(image_from_dict(c['image']))
                else:
                    images.append(invert_image_from_dict(c['image']))
                conds.append(c['strength'])
            if len(images) == 1:
                images = images[0]
            if len(conds) == 1:
                conds = conds[0]
            additional_args['image'] = images
            additional_args['controlnet_conditioning_scale'] = conds
    elif pipeline_type == 'img2img':
        additional_args = {
            'image': image_from_dict(input_image),
            'strength': params['strength'],
        }
    elif pipeline_type == 'inpaint2img':
        image = image_from_dict(input_image)
        if params.get("invert_mask"):
            mask = invert_image_from_dict(input_mask)
        else:
            mask = image_from_dict(input_mask)
        additional_args = {
            'image': image,
            'mask_image': mask,
            'width': width,
            'height': height,
        }

    pipeline.to(device)
    latents_noise.to(device)
    with torch.inference_mode(), torch.autocast("cuda"):
        result = pipeline(
            prompt, 
            negative_prompt=negative, 
            guidance_scale=cfg, 
            num_inference_steps=steps,
            generator=generator,
            callback=progress_preview,
            **additional_args,
        ).images[0]
    
    report("image generated")
    return {
            'data': result.tobytes(),
            'width': result.width,
            'height': result.height,
            'mode': result.mode,
        }


def run_pipeline(mode: str, params: dict):
    progress(0, 100, None)

    try:
        data = _run_pipeline(mode, params)   
    except CancelException:
        print("Image generation canceled")
        data = None
        gc.collect()

    progress(100, 100, None)     

    return data


def txt2img(params: dict):
    return run_pipeline('txt2img', params)


def img2img(params: dict):
    return run_pipeline('img2img', params)
