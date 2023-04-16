import gc

import torch
from models.models import create_pipeline
from images.latents import create_latents_noise, latents_to_pil
from exceptions.exceptions import CancelException
from dexpert import progress, progress_canceled, progress_title



def report(message):
    progress_title(f'[Text To Image] - {message}')


def txt2img(params: dict):
    device = "cuda"

    @torch.no_grad()
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
        if width % 8 != 0:
            width += 8 - width % 8
        if height % 8 != 0:
            height += 8 - height % 8
        variation_enabled = params.get('var_stren', 0) > 0
        var_stren = params.get("var_stren", 0)
        subseed = params['variation'] if variation_enabled else None
        report("creating the pipeline")
        pipeline = create_pipeline(model) 
        shape = (4, height // 8, width // 8 )
        latents_noise = create_latents_noise(shape, seed, subseed, var_stren)
        pipeline.to(device)
        generator = None if seed == -1  else torch.Generator(device="cuda").manual_seed(seed)
        report("generating the variation" if variation_enabled else "generating the image")

        def progress_preview(step, timestep, latents):
            progress(step, steps, latents_to_pil(step, pipeline.vae, latents))
            if progress_canceled():
                raise CancelException()
        
        latents_noise.to(device)

        with torch.inference_mode(), torch.autocast("cuda"):
            result = pipeline(
                prompt, 
                guidance_scale=cfg, 
                width=width, 
                height=height, 
                negative_prompt=negative, 
                num_inference_steps=steps,
                generator=generator,
                latents=latents_noise,
                callback=lambda step, timest, latents: progress_preview(step, timest, latents)
            ).images[0]
        
        report("image generated")
        return {
                'data': result.tobytes(),
                'width': result.width,
                'height': result.height,
                'mode': result.mode,
            }
    
    progress(0, 100, None)

    try:
        data = do_it()   
    except CancelException:
        print("Image generation canceled")
        data = None
        gc.collect()

    progress(100, 100, None)     

    return data
