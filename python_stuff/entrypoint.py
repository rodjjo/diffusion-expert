import os
from images.diffusion_routines import run_pipeline
from utils.settings import set_user_settings, get_setting
from images.pre_process import pre_process_image
from images.filesystem import save_image, open_image
from models.my_gfpgan import gfpgan_restore_faces, gfpgan_upscale
from models.models import get_sd_model_urls, download_sd_model, get_embeddings
from models.my_interrogate import inerrogate_clip


MODEL_EXTENSIONS = set(['.ckpt', '.safetensors'])

def txt2img(params: dict):
    return run_pipeline('txt2img', params)


def img2img(params: dict):
    return run_pipeline('img2img', params)


def is_model(path: str):
    n = path.lower()
    for e in MODEL_EXTENSIONS:
        if n.endswith(e):
            return True
    return False


def _list_models(directory=""):
    if (directory == ''):
        return []
    files = [n for n in os.listdir(directory) if is_model(n)]
    path = lambda n: os.path.join(directory, n)
    return [{
        "path": path(n),
        "name": n,
        "size": os.stat(path(n)).st_size,
        "hash": "not-computed",
    } for n in files]


def list_models(directory=""):
   return _list_models(directory) + _list_models(get_setting('add_model_dir', ''))


if __name__ == '__main__':
    print('models module loaded')