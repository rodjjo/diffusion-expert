import gc

import torch
from torchvision import transforms
from PIL import Image
from utils.images import pil_as_dict


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
        return {}
    '''
    Function to convert latents to images
    '''
    latents = (1 / 0.18215) * latents
    with torch.no_grad():
        image = vae.decode(latents).sample
    image = (image / 2 + 0.5).clamp(0, 1)
    image = image.detach().cpu().permute(0, 2, 3, 1).numpy()
    images = (image * 255).round().astype("uint8")
    if (len(images) < 1):
        return {}
    image = Image.fromarray(images[-1]) 
    return pil_as_dict(image)


def randn(seed, shape):
    torch.manual_seed(seed)
    return torch.randn(shape, device='cuda')


def randn_without_seed(shape):
    return torch.randn(shape, device='cuda')


# from https://discuss.pytorch.org/t/help-regarding-slerp-function-for-generative-model-sampling/32475/3
def slerp(val, low, high):
    low_norm = low/torch.norm(low, dim=1, keepdim=True)
    high_norm = high/torch.norm(high, dim=1, keepdim=True)
    dot = (low_norm * high_norm).sum(1)

    if dot.mean() > 0.9995:
        return low * val + high * (1 - val)

    omega = torch.acos(dot)
    so = torch.sin(omega)
    res = (torch.sin((1.0-val) * omega)/so).unsqueeze(1) * low + (torch.sin(val*omega)/so).unsqueeze(1) * high
    return res


def create_latents_noise(shape, seed, subseed=None, subseed_strength=0.0):
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