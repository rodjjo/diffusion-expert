from typing import List
from dfe.models.loader import load_state_dict, load_unet, load_vae, load_tokenizer, load_tiny_vae, load_scheduler
from dfe.images.routines import pil_as_dict
from dfe.misc.config import get_model_path

from diffusers import StableDiffusionPipeline
import torch
# {'positive_prompt': '', 'negative_prompt': '', 'mode': 'txt2img', 'model': 'addictivefuture_v1.safetensors', 
# 'inpaint_model': 'beautifulArt_v70.inpainting.safetensors', 'cfg': 7.5, 'seed': -1, 'width': 512, 'height': 512, 'steps': 25, 
# 'batch_size': 1, 'image': {}, 'mask': {}, 'controlnets': []}

INPAINT_KEY = {
    True: "inpaint",
    False: "normal"
}

MODEL_CACHE = {
    INPAINT_KEY[True] : {},
    INPAINT_KEY[False] : {},
    'vae': {},
    'tiny_vae': {},
}

PIPELINE_CACHE = {
    INPAINT_KEY[True] : {},
    INPAINT_KEY[False] : {},
}

class LoadedModel:
    unet: object
    vae: object
    tiny_vae: object
    text_model: object
    tokenizer: object
    config: object
    unet_config: object

    def __init__(self, unet, vae, tiny_vae, text_model, tokenizer, config, unet_config: object) -> None:
        self.unet = unet
        self.vae = vae
        self.tiny_vae = tiny_vae
        self.text_model = text_model
        self.tokenizer = tokenizer
        self.config = config
        self.unet_config = unet_config


def cached_load_unet(
    model: str, 
    inpaint: bool, 
    lora_list: List[dict], 
    use_tiny_vae: bool,
    use_float16: bool,
    keep_in_memory: bool
) -> LoadedModel:
    cache = MODEL_CACHE[INPAINT_KEY[inpaint]]
    if cache.get('model_file', '') != model or cache.get('use_fp16', True) != use_float16:
        cache.clear()
        cache['model_file'] = model
        cache['use_fp16'] = use_float16

    if cache.get('lora_list', []) != lora_list:
        cache['unet'] = None
        cache['lora_list'] = lora_list

    unet = cache.get('unet')
    if unet:
        return LoadedModel(
            unet=unet,
            vae=MODEL_CACHE['vae']['model'],
            tiny_vae=MODEL_CACHE.get('tiny_vae', {}).get('model'),
            text_model=MODEL_CACHE['text_model'],
            tokenizer=MODEL_CACHE['tokenizer']['model'],
            config=MODEL_CACHE['config'],
            unet_config=MODEL_CACHE['unet_config'],
        )
    state_dict = cache.get('state_dict')
    if state_dict is None:
        state_dict = load_state_dict(model)
        if keep_in_memory:
            cache['state_dict'] = state_dict

    MODEL_CACHE['text_model'] = state_dict.text_model
    MODEL_CACHE['config'] = state_dict.config
    MODEL_CACHE['unet_config'] = state_dict.unet_config

    if MODEL_CACHE.get('vae', {}).get('type') != state_dict.kind:
        MODEL_CACHE['vae'] = {}
        MODEL_CACHE['vae']['type'] = state_dict.kind

    vae = MODEL_CACHE.get('vae', {}).get('model') or load_vae(state_dict, use_float16)
    MODEL_CACHE['vae']['model'] = vae

    if MODEL_CACHE.get('tokenizer', {}).get('type') != state_dict.kind:
        MODEL_CACHE['tokenizer'] = {}
        MODEL_CACHE['tokenizer']['type'] = state_dict.kind

    tokenizer = MODEL_CACHE.get('tokenizer', {}).get('model') or load_tokenizer(state_dict)
    MODEL_CACHE['tokenizer']['model'] = tokenizer

    if use_tiny_vae:
        if MODEL_CACHE.get('tiny_vae', {}).get('type') != state_dict.kind:
            MODEL_CACHE['tiny_vae'] = {}
            MODEL_CACHE['tiny_vae']['type'] = state_dict.kind
        MODEL_CACHE['tiny_vae']['model'] = load_tiny_vae(state_dict, use_float16)

    unet = load_unet(state_dict, use_float16)
    cache['unet'] = unet

    return LoadedModel(
            unet=cache['unet'],
            vae=MODEL_CACHE['vae']['model'],
            tiny_vae=MODEL_CACHE.get('tiny_vae', {}).get('model'),
            text_model=MODEL_CACHE['text_model'],
            tokenizer=MODEL_CACHE['tokenizer']['model'],
            config=MODEL_CACHE['config'],
            unet_config=MODEL_CACHE['unet_config']
        )


def create_pipeline(mode: str, scheduler_name: str, model: LoadedModel):
    scheduler = load_scheduler(scheduler_name, config=model.config)
    safety_checker = None
    feature_extractor = None
    requires_safety_checker = False
    if mode == 'txt2img' or True:
        model.unet.to('cuda')
        pipe = StableDiffusionPipeline(
            vae=model.vae,
            text_encoder=model.text_model,
            tokenizer=model.tokenizer,
            unet=model.unet,
            scheduler=scheduler,
            safety_checker=safety_checker,
            feature_extractor=feature_extractor,
            requires_safety_checker=requires_safety_checker
        )
    if model.tiny_vae:
        pipe.vae = model.tiny_vae
    pipe.to('cuda')

    return pipe

def execute_pipeline(pipeline, prompt, negative, cfg, steps, seed, width, height, batch_size):
    with torch.inference_mode(), torch.autocast('cuda'):
        result = pipeline(
            prompt, 
            seed=seed,
            negative_prompt=negative, 
            guidance_scale=cfg, 
            num_inference_steps=steps,
            width=width,
            height=height
        )
        return result.images

def generate_internal(
    mode: str, 
    positive_prompt: str, 
    negative_prompt: str,
    model: str, 
    inpaint_model: str,
    scheduler_name: str,
    cfg: float,
    seed: int,
    width: int,
    height: int,
    steps: int,
    batch_size: int,
    image: dict,
    mask: dict,
    controlnets: List[dict],
    lora_list: List[dict],
    use_lcm_lora: bool,
    use_tiny_vae: bool,
    keep_in_memory: bool,
    filter_nsftw: bool,
    use_float16: bool
) -> List[dict]:
    inpaint = mode != 'txt2img'
    loaded_model = cached_load_unet(
        inpaint_model if inpaint else model, 
        inpaint,
        lora_list,
        use_tiny_vae, 
        use_float16,
        keep_in_memory
    )
    pipeline = create_pipeline(mode, scheduler_name, loaded_model)
    result = execute_pipeline(
        pipeline=pipeline,
        seed=seed, 
        width=width, 
        height=height,
        prompt=positive_prompt,
        negative=negative_prompt,
        cfg=cfg,
        steps=steps,
        batch_size=batch_size
    )
    return [
        pil_as_dict(r) for r in result
    ]


def generate(params: dict) -> dict:
    return generate_internal(
        positive_prompt=params['positive_prompt'], 
        negative_prompt=params['negative_prompt'], 
        mode=params.get('mode', "txt2img"), 
        model=get_model_path(params["model"]), 
        inpaint_model=get_model_path(params.get("inpaint_model", None)),
        scheduler_name=params.get('scheduler_name', 'EulerAncestralDiscreteScheduler'),
        cfg=params["cfg"],
        seed=params.get("seed", -1),
        width=params.get("width", 512),
        height=params.get("height", 512),
        steps=params.get("steps", 25),
        batch_size=params.get("batch_size", 1),
        image=params.get("image", {}),
        mask=params.get("mask", {}),
        controlnets=params.get("controlnets", []),
        lora_list=params.get("lora_list", []),
        use_lcm_lora=params.get("use_lcm_lora", False),
        use_tiny_vae=params.get("use_tiny_vae", False),
        keep_in_memory=params.get("keep_in_memory", False),
        filter_nsftw=params.get("filter_nsftw", False),
        use_float16=params.get("use_float16", False)
    )
    return [rgb_image_512x512()]
