import gc
import re
import torch
import os
from models.models import create_pipeline, current_model_is_in_painting, models_memory_checker
from images.latents import create_latents_noise, latents_to_pil
from exceptions.exceptions import CancelException
from utils.settings import get_setting
from utils.images import pil_as_dict, pil_from_dict, inpaint_fill_image
from models.my_gfpgan import gfpgan_dwonload_model, gfpgan_restore_faces
from models.paths import LORA_DIR

from dexpert import progress, progress_canceled, progress_title


REPORT_PREFIX = 'Text To Image'
REPORT_PREFIXES = {
    'txt2img': 'Text to Image',
    'inpaint2img': 'Inpaint',
    'img2img': 'Image to Image',
}

def set_prefix(text):
    global REPORT_PREFIX
    REPORT_PREFIX = text

def report(message):
    progress_title(f'[{REPORT_PREFIX}] - {message}')


def get_lora_path(lora: str) -> str:
    for d in (LORA_DIR, get_setting('add_lora_dir', '')):
        for e in ('.safetensors', '.ckpt'):
            filepath = os.path.join(d, f'{lora}{e}')
            if os.path.exists(filepath):
                return filepath
    return None


def parse_prompt_loras(prompt: str):
    lora_re = re.compile('<lora:[^:]+:[^>]+>')
    lora_list = re.findall(lora_re, prompt)

    lora_items = []
    for lora in lora_list:
        lora = lora.replace('<', '').replace('>', '')
        p = lora.split(':')
        if len(p) != 3:
            continue
        p = [p[1], p[2]]
        try:
            weight = float(p[1])
        except Exception:
            report(f"Invalid lora weigth {p[1]}")
            continue
        filepath = get_lora_path(p[0])
        if not filepath:
            report(f"Lora not found: {p[0]}")
            continue
        lora_items.append([filepath, weight])
    return re.sub(lora_re, '', prompt), lora_items


@torch.no_grad()
def _run_pipeline(pipeline_type, params):
    device = get_setting('device', 'cuda')
    restore_faces = params.get('restore_faces')
    if restore_faces:
        gfpgan_dwonload_model()
        restore_faces = True

    prompt, lora_list = parse_prompt_loras(params['prompt'])
    negative = params['negative']

    if len(negative or '') < 2:
        negative = None

    seed = params['seed']
    model = params["model"]
    cfg = params["cfg"]
    steps = params["steps"]
    width = params["width"]
    height = params["height"]
    reload_model = params.get("reload_model", False) 
    input_image = params.get("image")
    input_mask = params.get("mask")
    inpaint_mode = params.get("inpaint_mode", "original")
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
    
    generator = None if seed == -1  else torch.Generator(device=device).manual_seed(seed)

    if pipeline_type == 'img2img' and input_mask is not None:
        pipeline_type = 'inpaint2img'

    set_prefix(REPORT_PREFIXES.get(pipeline_type, "Text to Image"))

    report("started")

    report("creating the pipeline")
    pipeline = create_pipeline(pipeline_type, model, controlnets=controlnets, lora_list=lora_list, reload_model=reload_model) 
    report("pipeline created")

    if pipeline_type == 'inpaint2img':
        if not current_model_is_in_painting():
            return {
                "error": "The current model is not for inpainting"
            }
    elif current_model_is_in_painting():
        return {
            "error": "The current model is a inpainting model"
        }

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
                images.append(pil_from_dict(c['image']))
                conds.append(c['strength'])
            if len(images) == 1:
                images = images[0]
            if len(conds) == 1:
                conds = conds[0]
            additional_args['image'] = images
            additional_args['controlnet_conditioning_scale'] = conds
    elif pipeline_type == 'img2img':
        additional_args = {
            'image': pil_from_dict(input_image),
            'strength': params['strength'],
        }
        if len(controlnets):
            additional_args['width'] = width
            additional_args['height'] = height
            images = []
            conds = []
            for c in controlnets:
                if c['strength'] < 0:
                    c['strength'] = 0
                if c['strength'] > 2.0:
                    c['strength'] = 2.0
                images.append(pil_from_dict(c['image']))
                conds.append(c['strength'])
            if len(images) == 1:
                images = images[0]
            if len(conds) == 1:
                conds = conds[0]
            additional_args['controlnet_conditioning_image'] = images
            additional_args['controlnet_conditioning_scale'] = conds
    elif pipeline_type == 'inpaint2img':
        if not current_model_is_in_painting():
            return {
                "error": "The current model is not for in painting"
            }
        image = pil_from_dict(input_image)
        mask = pil_from_dict(input_mask)
        
        if inpaint_mode != 'original':
            image = inpaint_fill_image(image, mask)

        additional_args = {
            'image': image,
            'mask_image': mask,
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
                images.append(pil_from_dict(c['image']))
                conds.append(c['strength'])
            if len(images) == 1:
                images = images[0]
            if len(conds) == 1:
                conds = conds[0]
            additional_args['controlnet_conditioning_image'] = images
            additional_args['controlnet_conditioning_scale'] = conds

    pipeline.to(device)
    latents_noise.to(device)
    report("generating the variation" if variation_enabled else "generating the image")
    with torch.inference_mode(), torch.autocast(device):
        result = pipeline(
            prompt, 
            negative_prompt=negative, 
            guidance_scale=cfg, 
            num_inference_steps=steps,
            generator=generator,
            callback=progress_preview,
            **additional_args,
        ).images[0]

    if restore_faces:
        progress(99, 100, pil_as_dict(result)) 
        result = gfpgan_restore_faces(result)

    report("image generated")
    return pil_as_dict(result)


def run_pipeline(mode: str, params: dict):
    progress(0, 100, {})

    try:
        data = _run_pipeline(mode, params)   
    except CancelException:
        print("Image generation canceled")
        data = {"error": "Operation canceled by the user"}
    gc.collect()

    progress(100, 100, {})     

    return data

