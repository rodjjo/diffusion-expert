import os
import safetensors
import torch
from omegaconf import OmegaConf
from collections import defaultdict

from diffusers import (
    AutoencoderKL,
    AutoencoderTiny,
    PNDMScheduler,
    DDIMScheduler,
    EulerDiscreteScheduler,
    EulerAncestralDiscreteScheduler,
    LMSDiscreteScheduler,
    UniPCMultistepScheduler,
    LCMScheduler,

    UNet2DConditionModel
)

from transformers import CLIPTokenizer, CLIPTextModel
from dfe.misc.config import CONFIG_DIR, VAES_DIR, CACHE_DIR, get_lora_location
from dfe.models.sd15unet_conv import convertsd15_checkpoint, convert_ldm_clip_checkpoint, create_unet_diffusers_config


usefp16 = {
    True: torch.float16,
    False: torch.float32
}

MODEL_SD15          = "SD1_5"
MODEL_SD20          = "SD20"
MODEL_SDXL          = "SDXL"
MODEL_SDXL_TURBO    = "SDXLTURBO"
MODEL_LCM15         = "LCM15"
MODEL_LCM20         = "LCM20"
MODEL_LCMXL         = "LCMXL"
MODEL_SSD1B         = "SSD_1B"


SD1_5_REPLACEMENTS = {
    'cond_stage_model.transformer.embeddings.': 'cond_stage_model.transformer.text_model.embeddings.',
    'cond_stage_model.transformer.encoder.': 'cond_stage_model.transformer.text_model.encoder.',
    'cond_stage_model.transformer.final_layer_norm.': 'cond_stage_model.transformer.text_model.final_layer_norm.',
}

class StateDictInfo:
    state: dict
    kind: str
    inpaint: bool
    config: object
    unet_config: object
    text_model: object

    def __init__(self, 
                 state: dict, 
                 kind: str, 
                 inpaint: bool, 
                 config: object, 
                 text_model: object,
                 unet_config: object
    ) -> None:
        self.state = state
        self.kind = kind
        self.inpaint = inpaint
        self.config = config
        self.text_model = text_model
        self.unet_config = unet_config


def unet_config_path(kind: str, inpaint: bool) -> str:
    if kind == MODEL_SD15:
        if inpaint:
            return os.path.join(CONFIG_DIR, 'v1-inpainting-inference.yaml')
        else:
            return os.path.join(CONFIG_DIR, 'v1-inference.yaml')
    return ""


def vae_config_path(kind: str) -> str:
    if kind == MODEL_SD15:
        return os.path.join(CONFIG_DIR, 'v1-vae.config')
    return ""


def detect_kind(checkpoint: dict):
    #if 'time_embed.0.weight' not in checkpoint:
    #    return MODEL_SDXL
    return MODEL_SD15


def load_state_dict(path: str):
    if path.lower().endswith('.safetensors'):
        checkpoint = safetensors.torch.load_file(path, device="cpu")
    else:
        checkpoint = torch.load(path, map_location="cpu")

    checkpoint = checkpoint.pop("state_dict", checkpoint)
    checkpoint.pop("state_dict", None)
    kind = detect_kind(checkpoint) # TODO: detect model type
    inpaint = False
    text_model = None

    if kind == MODEL_SD15:
        b = checkpoint.get('model.diffusion_model.input_blocks.0.0.weight')
        if b is not None and b.shape[1] == 9:
            inpaint = True
        config = OmegaConf.load(unet_config_path(kind, inpaint))
        unet_config = create_unet_diffusers_config(config)

        result = {}
        for k, v in checkpoint.items():
            for text, replacement in SD1_5_REPLACEMENTS.items():
                if k.startswith(text):
                    k = replacement + k[len(text):]
                    break
            result[k] = v
        checkpoint.clear()
        checkpoint.update(result)
        text_model = convert_ldm_clip_checkpoint(checkpoint)
        checkpoint = convertsd15_checkpoint(checkpoint, unet_config)
    
    return StateDictInfo(checkpoint, kind, inpaint, config, text_model, unet_config)

def load_text_model(text_model_dict: dict) -> CLIPTextModel:
    local_files_only = False
    if os.path.exists(os.path.join(CACHE_DIR, 'models--openai--clip-vit-large-patch14', 'snapshots')):
        local_files_only = True
    text_model = CLIPTextModel.from_pretrained("openai/clip-vit-large-patch14", cache_dir=CACHE_DIR, local_files_only=local_files_only)
    text_model.load_state_dict(text_model_dict)
    return text_model

def load_unet(info: StateDictInfo, use_float16: bool):
    state = info.state
    if info.kind == MODEL_SD15:
        unet = UNet2DConditionModel(**info.unet_config)
        unet.load_state_dict(state, strict=True)
        if use_float16:
            unet = unet.half()
        return unet
    return None


def load_vae(info: StateDictInfo, use_float16: bool): 
    if info.kind in (MODEL_SD15, MODEL_LCM15):
        vae_path = os.path.join(VAES_DIR, 'sd15.vae.safetensors')
        #if not os.path.exists(vae_path):
        #    raise Exception(f"Vae file not found! Path: {vae_path}" )
        # https://huggingface.co/CompVis/stable-diffusion-v1-4/resolve/main/vae/diffusion_pytorch_model.safetensors?download=true
        #vae = AutoencoderKL.from_pretrained(
        #    vae_path, 
        #    config=vae_config_path(info.kind),
        #    local_files_only=True
        #)
        # vae = 
        vae = AutoencoderKL.from_pretrained(
            'CompVis/stable-diffusion-v1-4', 
            subfolder="vae", 
            torch_dtype=usefp16[use_float16], 
            cache_dir=CACHE_DIR
        )
        return vae
    return None


def load_tokenizer(info: StateDictInfo):
    if info.kind in (MODEL_SD15, MODEL_LCM15):
        return CLIPTokenizer.from_pretrained("openai/clip-vit-large-patch14", cache_dir=CACHE_DIR)


def load_tiny_vae(info: StateDictInfo, use_float16: bool):
    if info.kind in (MODEL_SD15, MODEL_LCM15):
        return AutoencoderTiny.from_pretrained(
            "madebyollin/taesd", 
            cache_dir=CACHE_DIR, 
            torch_dtype=usefp16[use_float16])
    else:
        return AutoencoderTiny.from_pretrained(
            "madebyollin/taesdxl", 
            cache_dir=CACHE_DIR, 
            torch_dtype=usefp16[use_float16])
    
def load_scheduler(name: str, config: object):
    num_train_timesteps = config.model.params.timesteps
    beta_start = config.model.params.linear_start
    beta_end = config.model.params.linear_end
    if name == 'LCMScheduler':
        scheduler = LCMScheduler(
            beta_start=beta_start,
            beta_end=beta_end,
            beta_schedule="scaled_linear",
            clip_sample=False,
            set_alpha_to_one=False,
            prediction_type="epsilon",
            steps_offset=1
        ) 
    elif name == 'EulerAncestralDiscreteScheduler':
        scheduler = EulerAncestralDiscreteScheduler(
            beta_start=beta_start,
            beta_end=beta_end,
            beta_schedule="scaled_linear",
            prediction_type="epsilon",
            steps_offset=1
        )
    elif name == 'DDIMScheduler':
        scheduler = DDIMScheduler(
            beta_start=beta_start,
            beta_end=beta_end,
            beta_schedule="scaled_linear",
            clip_sample=False,
            set_alpha_to_one=False,
            steps_offset=1
        )
    elif name == 'PNDMScheduler': 
        scheduler = PNDMScheduler(
            beta_start=beta_start,
            beta_end=beta_end,
            beta_schedule="scaled_linear",
            num_train_timesteps=num_train_timesteps,
            skip_prk_steps=True,
            steps_offset=1
        )
    elif  name == 'UniPCMultistepScheduler':
        scheduler = UniPCMultistepScheduler(
            beta_start=beta_start,
            beta_end=beta_end,
            beta_schedule="scaled_linear",
            steps_offset=1
            # clip_sample=False,
            # set_alpha_to_one=False
        )
    else:
         scheduler = LMSDiscreteScheduler(
            beta_start=beta_start, 
            beta_end=beta_end, 
            beta_schedule="scaled_linear",
            steps_offset=1
        )
    
    return scheduler


def load_lora_weights(unet, text_encoder, lora_name, lora_weight):
    lora_path = get_lora_location(lora_name)
    if lora_path is None:
        raise Exception(f"Lora not found: {lora_name}")

    LORA_PREFIX_UNET = "lora_unet"
    LORA_PREFIX_TEXT_ENCODER = "lora_te"

    # load LoRA weight from .safetensors
    if lora_path.lower().endswith('.safetensors'):
        state_dict = safetensors.torch.load_file(lora_path, device="cpu") 
    else:
        state_dict = torch.load(lora_path, map_location="cpu")

    updates = defaultdict(dict)
    for key, value in state_dict.items():
        # it is suggested to print out the key, it usually will be something like below
        # "lora_te_text_model_encoder_layers_0_self_attn_k_proj.lora_down.weight"

        layer, elem = key.split('.', 1)
        updates[layer][elem] = value

    # directly update weight in diffusers model
    for layer, elems in updates.items():

        if "text" in layer:
            layer_infos = layer.split(LORA_PREFIX_TEXT_ENCODER + "_")[-1].split("_")
            curr_layer = text_encoder
        else:
            layer_infos = layer.split(LORA_PREFIX_UNET + "_")[-1].split("_")
            curr_layer = unet

        # find the target layer
        temp_name = layer_infos.pop(0)
        while len(layer_infos) > -1:
            try:
                curr_layer = curr_layer.__getattr__(temp_name)
                if len(layer_infos) > 0:
                    temp_name = layer_infos.pop(0)
                elif len(layer_infos) == 0:
                    break
            except Exception:
                if len(temp_name) > 0:
                    temp_name += "_" + layer_infos.pop(0)
                else:
                    temp_name = layer_infos.pop(0)

        # get elements for this layer
        weight_up = elems['lora_up.weight'].to(torch.float32)
        weight_down = elems['lora_down.weight'].to(torch.float32)
        alpha = elems['alpha']
        if alpha:
            alpha = alpha.item() / weight_up.shape[1]
        else:
            alpha = 1.0

        if len(weight_up.shape) == 4:
            weight_up = weight_up.squeeze(3).squeeze(2).to(torch.float32)
            weight_down = weight_down.squeeze(3).squeeze(2).to(torch.float32)
            if len(weight_up.shape) == len(weight_down.shape):
                curr_layer.weight.data += lora_weight * alpha * torch.mm(weight_up, weight_down).unsqueeze(2).unsqueeze(3)
            else:
                curr_layer.weight.data += lora_weight * alpha * torch.einsum('a b, b c h w -> a c h w', weight_up, weight_down)       
        else:
            curr_layer.weight.data += lora_weight * alpha * torch.mm(weight_up, weight_down)

