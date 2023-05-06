import os
from utils.images import pil_from_dict
from utils.downloader import download_file
from models.paths import DEEPBOORU_DIR
from external.deepbooru import get_deepbooru_tags

from dexpert import progress_title


def download_deepbooru():
    model_path = DEEPBOORU_DIR
    if not os.path.exists(os.path.join(model_path, 'project.json')):
        download_file(
            'https://github.com/KichangKim/DeepDanbooru/releases/download/v3-20211112-sgd-e28/deepdanbooru-v3-20211112-sgd-e28.zip',
            DEEPBOORU_DIR,
            'deepdanbooru-v3-20211112-sgd-e28.zip'
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
