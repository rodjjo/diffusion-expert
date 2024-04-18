import os
from typing import Any, Optional, Union
import safetensors.torch
import torch
from ella.model import ELLA, T5TextEmbedder
from utils.downloader import download_file
from models.paths import ELLA_MODELS_DIR
from diffusers import DPMSolverMultistepScheduler

ELLA_FILE_NAME = 'ella-sd1.5-tsc-t5xl.safetensors'

def dowload_ella():
    download_file(
        'https://huggingface.co/QQGYLab/ELLA/resolve/main/ella-sd1.5-tsc-t5xl.safetensors',
        ELLA_MODELS_DIR, 
        ELLA_FILE_NAME
    )
   

class ELLAProxyUNet(torch.nn.Module):
    def __init__(self, ella, unet):
        super().__init__()
        # In order to still use the diffusers pipeline, including various workaround

        self.ella = ella
        self.unet = unet
        self.config = unet.config
        self.dtype = unet.dtype
        self.device = unet.device

        self.flexible_max_length_workaround = None

    def forward(
        self,
        sample: torch.FloatTensor,
        timestep: Union[torch.Tensor, float, int],
        encoder_hidden_states: torch.Tensor,
        class_labels: Optional[torch.Tensor] = None,
        timestep_cond: Optional[torch.Tensor] = None,
        attention_mask: Optional[torch.Tensor] = None,
        cross_attention_kwargs: Optional[dict[str, Any]] = None,
        added_cond_kwargs: Optional[dict[str, torch.Tensor]] = None,
        down_block_additional_residuals: Optional[tuple[torch.Tensor]] = None,
        mid_block_additional_residual: Optional[torch.Tensor] = None,
        down_intrablock_additional_residuals: Optional[tuple[torch.Tensor]] = None,
        encoder_attention_mask: Optional[torch.Tensor] = None,
        return_dict: bool = True,
    ):
        if self.flexible_max_length_workaround is not None:
            time_aware_encoder_hidden_state_list = []
            for i, max_length in enumerate(self.flexible_max_length_workaround):
                time_aware_encoder_hidden_state_list.append(
                    self.ella(encoder_hidden_states[i : i + 1, :max_length], timestep)
                )
            # No matter how many tokens are text features, the ella output must be 64 tokens.
            time_aware_encoder_hidden_states = torch.cat(
                time_aware_encoder_hidden_state_list, dim=0
            )
        else:
            time_aware_encoder_hidden_states = self.ella(
                encoder_hidden_states, timestep
            )

        return self.unet(
            sample=sample,
            timestep=timestep,
            encoder_hidden_states=time_aware_encoder_hidden_states,
            class_labels=class_labels,
            timestep_cond=timestep_cond,
            attention_mask=attention_mask,
            cross_attention_kwargs=cross_attention_kwargs,
            added_cond_kwargs=added_cond_kwargs,
            down_block_additional_residuals=down_block_additional_residuals,
            mid_block_additional_residual=mid_block_additional_residual,
            down_intrablock_additional_residuals=down_intrablock_additional_residuals,
            encoder_attention_mask=encoder_attention_mask,
            return_dict=return_dict,
        )


def generate_image_with_flexible_max_length(
    pipe, t5_encoder, prompt, fixed_negative=False, output_type="pt", **pipe_kwargs
):
    device = pipe.device
    dtype = pipe.dtype
    prompt = [prompt] if isinstance(prompt, str) else prompt
    batch_size = len(prompt)

    prompt_embeds = t5_encoder(prompt, max_length=None).to(device, dtype)
    negative_prompt_embeds = t5_encoder(
        [""] * batch_size, max_length=128 if fixed_negative else None
    ).to(device, dtype)

    # diffusers pipeline concatenate `prompt_embeds` too early...
    # https://github.com/huggingface/diffusers/blob/b6d7e31d10df675d86c6fe7838044712c6dca4e9/src/diffusers/pipelines/stable_diffusion/pipeline_stable_diffusion.py#L913
    pipe.unet.flexible_max_length_workaround = [
        negative_prompt_embeds.size(1)
    ] * batch_size + [prompt_embeds.size(1)] * batch_size

    max_length = max([prompt_embeds.size(1), negative_prompt_embeds.size(1)])
    b, _, d = prompt_embeds.shape
    prompt_embeds = torch.cat(
        [
            prompt_embeds,
            torch.zeros(
                (b, max_length - prompt_embeds.size(1), d), device=device, dtype=dtype
            ),
        ],
        dim=1,
    )
    negative_prompt_embeds = torch.cat(
        [
            negative_prompt_embeds,
            torch.zeros(
                (b, max_length - negative_prompt_embeds.size(1), d),
                device=device,
                dtype=dtype,
            ),
        ],
        dim=1,
    )

    images = pipe(
        prompt_embeds=prompt_embeds,
        negative_prompt_embeds=negative_prompt_embeds,
        **pipe_kwargs,
        output_type=output_type,
    ).images
    pipe.unet.flexible_max_length_workaround = None
    return images


def load_ella(filename, device, dtype):
    ella = ELLA()
    safetensors.torch.load_model(ella, filename, strict=True)
    ella.to(device, dtype=dtype)
    return ella


def load_ella_for_pipe(pipe, ella):
    pipe.unet = ELLAProxyUNet(ella, pipe.unet)
    pipe.ella_t5_encoder = T5TextEmbedder().to(pipe.device, dtype=torch.float16)
    pipe.scheduler = DPMSolverMultistepScheduler.from_config(pipe.scheduler.config)


def inject_ella(pipeline):
    if getattr(pipeline, 'ella_t5_encoder', None) is not None:
        return
    dowload_ella()
    ella_path = os.path.join(ELLA_MODELS_DIR, ELLA_FILE_NAME)
    ella = load_ella(ella_path, pipeline.device, pipeline.dtype)
    load_ella_for_pipe(pipeline, ella)


def offload_ella_for_pipe(pipe):
    if getattr(pipe, 'ella_t5_encoder', None) is not None:
        pipe.unet = pipe.unet.unet
        pipe.ella_t5_encoder = None


def create_ella_prompts(pipe, prompt: str, negative: str):
    if len(prompt.strip()) < 1:
        prompt = "any thing"
    if len(negative.strip()) < 1:
        negative = "ugly"
    prompt = [prompt] 
    negative = [negative]
    prompt_embeds = pipe.ella_t5_encoder(prompt, max_length=128).to(pipe.device, pipe.dtype)
    negative_prompt_embeds = pipe.ella_t5_encoder(negative, max_length=128).to(pipe.device, pipe.dtype)
    return prompt_embeds, negative_prompt_embeds
