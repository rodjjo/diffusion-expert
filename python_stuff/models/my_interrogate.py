import os
from utils.images import pil_from_dict
from utils.downloader import download_file
from models.paths import DEEPBOORU_DIR
from external.deepdanbooru import get_deepbooru_tags

from dexpert import progress_title


def download_deepbooru():
    if not os.path.exists(os.path.join(DEEPBOORU_DIR, 'model-resnet_custom_v3.pt')):
        download_file(
            'https://github.com/AUTOMATIC1111/TorchDeepDanbooru/releases/download/v1/model-resnet_custom_v3.pt',
            DEEPBOORU_DIR,
            'model-resnet_custom_v3.pt'
        )


def inerrogate_clip(model, image) -> dict:
    if model == 'Clip':
        pass
    elif model == 'DeepBooru':
        download_deepbooru()
        progress_title("[DeepBooru] Interrogating the clip, wait...")
        return {
            'prompt': get_deepbooru_tags(pil_from_dict(image))
        }
    return {
        "error": "Not implemented yet"
    }
