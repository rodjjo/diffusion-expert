# from safetensors import safe_open
import os
import sys
import gc

import torch
from torchvision import transforms
from PIL import Image
from models.models import create_pipeline, create_var_pipeline
from dexpert import progress, progress_canceled, progress_title


def report(message):
    progress_title(f'[Text To Image] - {message}')
    
class CancelException(Exception):
    pass

def pil_to_latents(image, vae):
    '''     
    Function to convert image to latents     
    '''     
    init_image = transforms.ToTensor()(image).unsqueeze(0) * 2.0 - 1.0   
    init_image = init_image.to(device="cuda") # , dtype=torch.float16)
    init_latent_dist = vae.encode(init_image).latent_dist.sample() * 0.18215     
    return init_latent_dist

def latents_to_pil(step, vae, latents):
    if step % 5 != 0:
        return None
    '''
    Function to convert latents to images
    '''
    latents = (1 / 0.18215) * latents
    with torch.no_grad():
        image = vae.decode(latents).sample
    image = (image / 2 + 0.5).clamp(0, 1)
    image = image.detach().cpu().permute(0, 2, 3, 1).numpy()
    images = (image * 255).round().astype("uint8")
    if (len(images)):
        image = images[-1]
    image = Image.fromarray(image) 
    return {
        'data': image.tobytes(),
        'width': image.width,
        'height': image.height,
        'mode': image.mode,
    }


def randn(seed, shape):
    torch.manual_seed(seed)
    return torch.randn(shape, device='cuda')


def randn_without_seed(shape):
    return torch.randn(shape, device='cuda')


# from https://discuss.pytorch.org/t/help-regarding-slerp-function-for-generative-model-sampling/32475/3
def slerp(val, low, high):
    low_norm = low/torch.norm(low, dim=1, keepdim=True)
    high_norm = high/torch.norm(high, dim=1, keepdim=True)
    dot = (low_norm*high_norm).sum(1)

    if dot.mean() > 0.9995:
        return low * val + high * (1 - val)

    omega = torch.acos(dot)
    so = torch.sin(omega)
    res = (torch.sin((1.0-val)*omega)/so).unsqueeze(1)*low + (torch.sin(val*omega)/so).unsqueeze(1) * high
    return res


def create_random_tensors(shape, seed, subseed=None, subseed_strength=0.0):
    xs = []
    subnoise = None
    if subseed is not None:
        subnoise = randn(subseed, shape)
    # randn results depend on device; gpu and cpu get different results for same seed;
    # the way I see it, it's better to do this on CPU, so that everyone gets same result;
    # but the original script had it like this, so I do not dare change it for now because
    # it will break everyone's seeds.
    noise = randn(seed, shape)

    if subnoise is not None:
        noise = slerp(subseed_strength, noise, subnoise)

    xs.append(noise)
    x = torch.stack(xs).to('cuda')
    return x


def txt2img(params: dict):
    device = "cuda"
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
        variation_enabled = params.get('var_stren', 0) > 0
        var_stren = params.get("var_stren", 0)
        report("creating the pipeline")
        subseed = params['variation'] if variation_enabled else None
        pipeline = create_pipeline(model) 
        shape = (4, width // 8, height // 8)
        x = create_random_tensors(shape, seed, subseed, var_stren)
        pipeline.to(device)
        generator = None if seed == -1  else torch.Generator(device="cuda").manual_seed(seed)
        report("generating the variation" if variation_enabled else "generating the image")

        def progress_preview(step, timestep, latents):
            progress(step, steps, latents_to_pil(step, pipeline.vae, latents))
            if progress_canceled():
                raise CancelException()

        result = pipeline(
            prompt, 
            guidance_scale=cfg, 
            height=height, 
            width=width, 
            negative_prompt=negative, 
            num_inference_steps=steps,
            generator=generator,
            latents=x,
            callback=lambda step, timest, latents: progress_preview(step, steps, latents)
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
