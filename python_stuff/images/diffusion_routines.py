import gc
import re
import torch
import os
import numpy as np
from datetime import datetime
from models.models import create_pipeline,  create_cascade_pipeline
from images.latents import create_latents_noise, latents_to_pil, latents_to_pil_image
from exceptions.exceptions import CancelException
from utils.settings import get_setting
from utils.images import pil_as_dict, pil_from_dict, inpaint_fill_image, inpaint_noise
from models.my_gfpgan import gfpgan_dwonload_model, gfpgan_restore_faces
from models.paths import LORA_DIR
from PIL import Image
from torchvision.transforms.functional import pil_to_tensor


from dexpert import progress, progress_canceled, progress_title


REPORT_PREFIX = 'Text To Image'
REPORT_PREFIXES = {
    'txt2img': 'Text to Image',
    'inpaint2img': 'Inpaint',
    'img2img': 'Image to Image',
}

usefp16 = {
    True: torch.float16,
    False: torch.float32
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


def make_inpaint_condition(image, image_mask):
    image = np.array(image.convert("RGB")).astype(np.float32) / 255.0
    image_mask = np.array(image_mask.convert("L")).astype(np.float32) / 255.0

    assert image.shape[0:1] == image_mask.shape[0:1], "image and image_mask must have the same image size"
    image[image_mask > 0.5] = -1.0  # set as masked pixel
    image = np.expand_dims(image, 0).transpose(0, 3, 1, 2)
    image = torch.from_numpy(image)
    return image


def create_ipadapter_embds(pipeline, image):
    return pipeline.prepare_ip_adapter_image_embeds(
        ip_adapter_image=image,
        ip_adapter_image_embeds=None,
        device="cuda",
        num_images_per_prompt=1,
        do_classifier_free_guidance=True,
    )


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
    batch_size = params.get('batch_size', 1)
    reload_model = params.get("reload_model", False) 
    use_lcm = params.get("use_lcm", False) 
    free_lunch = params.get("free_lunch", False) 
    input_image = params.get("image")
    input_mask = params.get("mask")
    inpaint_mode = params.get("inpaint_mode", "original")
    controlnets = params.get("controlnets", [])
    face = params.get("face")
    adapter_image = params.get("adapter_image")

    if face:
        face = pil_from_dict(face)
    else:
        face = None

    if adapter_image:
        adapter_image = pil_from_dict(adapter_image)
    else:
        adapter_image = None

    if 'img2img' in pipeline_type  and input_mask is not None:
        pipeline_type = pipeline_type.replace('img2img', 'inpaint2img')

    if 'inpaint2img' in pipeline_type and  inpaint_mode == "img2img":
        pipeline_type = pipeline_type.replace('inpaint2img', 'img2img')

    if width % 8 != 0:
        width += 8 - width % 8

    if height % 8 != 0:
        height += 8 - height % 8

    variation_enabled = params.get('var_stren', 0) > 0
    var_stren = params.get("var_stren", 0)
    subseed = params['variation'] if variation_enabled else None
    
    shape = (4, height // 8, width // 8 )
    latents_noise = create_latents_noise(shape, seed, subseed, var_stren)
    latents_noise = latents_noise.to(dtype=usefp16[get_setting('use_float16', True)])
    
    generator = None if seed == -1  else [
        torch.Generator(device=device).manual_seed(seed + i)
        for i in range(batch_size)
    ]

    set_prefix(REPORT_PREFIXES.get(pipeline_type, "Text to Image"))

    report("started")

    report("creating the pipeline")
    leditpp = bool(input_image) and pipeline_type == 'txt2img'
    pipeline = create_pipeline(
        f'{"lcm_" if use_lcm else ""}{pipeline_type}', model, 
        controlnets=controlnets, 
        lora_list=lora_list, 
        reload_model=reload_model,
        free_lunch=free_lunch,
        face_image=face is not None,
        adapter_image=adapter_image is not None,
        leditpp=leditpp
    ) 
    report("pipeline created")

    '''
    if pipeline_type == 'inpaint2img':
        if not ):
            return [{
                "error": "The current model is not for inpainting"
            }]
    elif ):
        return [{
            "error": "The current model is a inpainting model"
        }]
    '''

    def progress_preview(step, timestep, latents):
        if use_lcm:
            progress(step, steps, {})
        else:
            progress(step, steps, latents_to_pil(step, pipeline.vae, latents))
        if progress_canceled():
            raise CancelException()

    additional_args = {
        'generator': generator
    }
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
        '''
        if not ):
            return [{
                "error": "The current model is not for in painting"
            }]
        '''
        image = pil_from_dict(input_image)
        mask = pil_from_dict(input_mask)
        
        if inpaint_mode != 'original' and inpaint_mode != 'img2img':
            if inpaint_mode == 'noise':
                temp = inpaint_noise(image, mask, latents_to_pil_image(0, pipeline.vae, latents_noise))
                if temp: 
                    image = temp
            else:
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
            has_inpaint = False
            for c in controlnets:
                if c['strength'] < 0:
                    c['strength'] = 0
                if c['strength'] > 2.0:
                    c['strength'] = 2.0
                if c['mode'] == 'inpaint':
                    images.append(make_inpaint_condition(image, mask))
                    has_inpaint = True
                else:
                    images.append(pil_from_dict(c['image']))
                conds.append(c['strength'])
            if len(images) == 1:
                images = images[0]
            if len(conds) == 1:
                conds = conds[0]
            if has_inpaint:
                additional_args['control_image'] = images
                additional_args['controlnet_conditioning_scale'] = conds
            else:
                additional_args['controlnet_conditioning_image'] = images
                additional_args['controlnet_conditioning_scale'] = conds


    latents_noise.to(device)
    with torch.inference_mode(), torch.autocast(device):
        additional_args['callback'] = progress_preview
        additional_args['callback_steps'] = 1

        if len(controlnets):
            batch_size = 1

        if type(pipeline.scheduler).__name__ == 'LCMScheduler' or 'sdxl turbo' in model.lower():
            if pipeline_type == 'txt2img':
                if cfg > 2:
                    cfg = 2
            elif pipeline_type == 'inpaint2img':
                if cfg > 4:
                    cfg = 4
            else:
                if cfg > 1:
                    cfg = 1

            if steps > 8:
                steps = 8

        if  batch_size > 1 and additional_args.get('latents') is not None:
            del additional_args['latents']

        if batch_size > 1:
            additional_args['batch_size'] = batch_size
            additional_args['num_images_per_prompt'] = batch_size
        
        if hasattr(pipeline, 'load_ip_adapter'):
            image_list_adapt = []
            if face:
                image_list_adapt  += [face]

            if adapter_image:
                image_list_adapt += [adapter_image]
            
            if image_list_adapt:
                additional_args["ip_adapter_image"] = image_list_adapt

        
        if leditpp:
            report("Inverting image")
            pipeline.invert(
                image=pil_from_dict(input_image),
                num_inversion_steps=steps,
                skip=0.1
            )
            report("generating the variation" if variation_enabled else "generating the image")
            result = pipeline(
                editing_prompt=[prompt], 
                # negative_prompt=[negative] if negative else [''],
                edit_guidance_scale=cfg, 
                edit_threshold=1.0 - params["strength"],
            ).images 
        else:
            report("generating the variation" if variation_enabled else "generating the image")
            result = pipeline(
                prompt, 
                negative_prompt=negative,
                guidance_scale=cfg, 
                num_inference_steps=steps,
                **additional_args,
            ).images 

    if restore_faces:
        for i, r in enumerate(result):
            progress(99, 100, pil_as_dict(r)) 
            result[i] = gfpgan_restore_faces(r)
    report("image generated")
    return [pil_as_dict(r) for r in result]


def _run_cascade(pipeline_type: str, params: dict):
    if 'txt2img' != pipeline_type:
        raise CancelException()
    restore_faces = params.get('restore_faces')
    if restore_faces:
        gfpgan_dwonload_model()
        restore_faces = True
    prompt, _ = parse_prompt_loras(params['prompt'])
    negative = params['negative']

    if len(negative or '') < 2:
        negative = None

    seed = params['seed']
    # model = params["model"]
    cfg = params["cfg"]
    steps = params["steps"]
    width = params["width"]
    height = params["height"]
    # batch_size = params.get('batch_size', 1)
    input_image = params.get("image")
    input_mask = None # params.get("mask")
    inpaint_mode = params.get("inpaint_mode", "original")


    if width % 8 != 0:
        width += 8 - width % 8

    if height % 8 != 0:
        height += 8 - height % 8
    
    report("Loading stable cascade model")
    prior, decoder = create_cascade_pipeline()
    
    with torch.inference_mode(), torch.autocast('cuda'):
        report("Generating the fist image")

        generator = None if seed == -1  else [
            torch.Generator(device='cuda').manual_seed(seed)
        ]
        # image = image.to(device=device, dtype=dtype)
        # image_embed = self.image_encoder(image).image_embeds.unsqueeze(1)
        if 'img2img' in pipeline_type and input_image is not None:
            prior_output = pil_to_tensor(pil_from_dict(input_image))
        else:
            prior.to('cuda')
            prior_output = prior(
                prompt=prompt,
                height=height,
                width=width,
                negative_prompt=negative,
                guidance_scale=cfg,
                generator=generator,
                num_images_per_prompt=1,
                num_inference_steps=steps,
            )
            prior.to('cpu')
            gc.collect()
            torch.cuda.empty_cache()
            prior_output = prior_output.image_embeddings.to(torch.float16)


        report("Generating the final")

        decoder.to('cuda')
        result = decoder(
            image_embeddings=prior_output,
            prompt=prompt,
            negative_prompt=negative,
            guidance_scale=0.0,
            output_type="pil",
            num_inference_steps=steps // 2
        ).images
        decoder.to('cpu')
        gc.collect()
        torch.cuda.empty_cache()


    if restore_faces:
        for i, r in enumerate(result):
            progress(99, 100, pil_as_dict(r)) 
            result[i] = gfpgan_restore_faces(r)

    return [pil_as_dict(r) for r in result]

def run_pipeline(mode: str, params: dict):
    progress(0, 100, {})

    try:
        time_start = datetime.utcnow()
        if 'cascade' in params["model"].lower():
            data = _run_cascade(mode, params)
        else:
            data = _run_pipeline(mode, params)   
        end_time = datetime.utcnow()
        dur = round((end_time - time_start).total_seconds(), 2)
        report(f"Image generation took {dur} seconds")
    except CancelException:
        print("Image generation canceled")
        data = {"error": "Operation canceled by the user"}
    
    torch.cuda.empty_cache()
    gc.collect()

    progress(100, 100, {})     

    return data

