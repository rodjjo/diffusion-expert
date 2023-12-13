from typing import List
from dfe.models.loader import (
    load_state_dict, 
    load_unet, 
    load_vae, 
    load_tokenizer, 
    load_tiny_vae, 
    load_scheduler, 
    load_text_model,
    load_lora_weights
)
from dfe.images.routines import pil_as_dict
from dfe.misc.config import get_model_path

from dexpert import progress, progress_canceled, progress_text

from diffusers import StableDiffusionPipeline
import torch

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

class CancelException(Exception):
    pass

def cached_load_unet(
    model: str, 
    inpaint: bool, 
    lora_list: List[dict], 
    use_lcm_lora: bool,
    use_tiny_vae: bool,
    use_float16: bool,
    keep_in_memory: bool
) -> LoadedModel:
    progress_text("Loading the model")
    cache = MODEL_CACHE[INPAINT_KEY[inpaint]]
    if cache.get('model_file', '') != model or cache.get('use_fp16', True) != use_float16:
        cache.clear()
        cache['model_file'] = model
        cache['use_fp16'] = use_float16

    if cache.get('lora_list', []) != lora_list or cache.get('use_lcm_lora') != use_lcm_lora:
        cache['unet'] = None
        cache['lora_list'] = lora_list
        cache['use_lcm_lora'] = use_lcm_lora

    unet = cache.get('unet')
    if unet:
        progress_text("Using cached model... not reloading from disk...")
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
        progress_text("Loading the model from disk...")
        state_dict = load_state_dict(model)
        if keep_in_memory:
            cache['state_dict'] = state_dict
    else:
        progress_text("Using preloaded state of the model...")

    MODEL_CACHE['config'] = state_dict.config
    MODEL_CACHE['unet_config'] = state_dict.unet_config

    if MODEL_CACHE.get('vae', {}).get('type') != state_dict.kind:
        MODEL_CACHE['vae'] = {}
        MODEL_CACHE['vae']['type'] = state_dict.kind

    vae = MODEL_CACHE.get('vae', {}).get('model') 
    if not vae:
        progress_text("Loading vae from disk...")
        vae = load_vae(state_dict, use_float16)
    else:
        progress_text("Using preloaded vae...")
        
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
        if MODEL_CACHE['tiny_vae'].get('model') is None:
            progress_text("Loading tiny vae from disk...")
            MODEL_CACHE['tiny_vae']['model'] = load_tiny_vae(state_dict, use_float16)
        else:
            progress_text("Using preloaded tiny vae...")
    
    progress_text("Loading unet from state dict...")
    unet = load_unet(state_dict, use_float16)
    MODEL_CACHE['text_model'] = load_text_model(state_dict.text_model)
    cache['unet'] = unet
    progress_text("Model loaded with success, applying loras...")
    # load lora list here

    if use_lcm_lora:
        progress_text("Using lcm lora (speed it up :))...")        
        load_lora_weights(unet, MODEL_CACHE['text_model'] , 'lcm-lora-sdv1-5', 1.0)

    progress_text("Loras applied with success...")        
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
    progress_text("Creating scheduler...")
    scheduler = load_scheduler(scheduler_name, config=model.config)
    safety_checker = None
    feature_extractor = None
    requires_safety_checker = False
    if mode == 'txt2img' or True:
        progress_text("Creating text to image pipeline...")
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
    def pipeline_callback(step, timestep, latents):
        progress(step, steps)
        if progress_canceled():
            raise CancelException()
    progress_text("Generating the image...")
    with torch.inference_mode(), torch.autocast('cuda'):
        result = pipeline(
            prompt, 
            seed=seed,
            negative_prompt=negative, 
            guidance_scale=cfg, 
            num_inference_steps=steps,
            width=width,
            height=height,
            callback=pipeline_callback,
            callback_steps=1
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
    progress_text(f"Generating image, mode: {mode} ...")
    inpaint = mode != 'txt2img'
    loaded_model = cached_load_unet(
        inpaint_model if inpaint else model, 
        inpaint,
        lora_list,
        use_lcm_lora,
        use_tiny_vae, 
        use_float16,
        keep_in_memory
    )
    pipeline = create_pipeline(mode, scheduler_name, loaded_model)
    try:
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
    except CancelException:
        return [
            {"error": "Canceled by the user"}
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
