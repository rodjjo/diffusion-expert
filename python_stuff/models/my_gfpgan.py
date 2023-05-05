import gc
import os
import sys
import urllib
import shutil
from contextlib import contextmanager

import numpy
import cv2
from gfpgan import GFPGANer  
from PIL import Image

from utils.settings import get_setting
from utils.images import pil_as_dict, pil_from_dict
from models.models import models_memory_checker

from exceptions.exceptions import CancelException

from dexpert import progress, progress_canceled, progress_title

UPSCALERS_DIR =  os.path.join(os.path.dirname(sys.executable), '..', 'models', 'upscalers')
BASE_DIR = os.path.join(UPSCALERS_DIR, 'gfpgan', 'weights')

# make sure it downloads the file inside the models_Dir
MODEL_FILE_NAME = 'GFPGANv1.4.pth'
MODEL_PATH = os.path.join(BASE_DIR, MODEL_FILE_NAME)


def report(message):
    progress_title(f'[GFPGAN Upscaler] - {message}')


def show_progress(block_num, block_size, total_size):
    progress(block_num * block_size, total_size, {})
    if progress_canceled():
        raise CancelException()


def gfpgan_dwonload_model():
    os.makedirs(BASE_DIR, exist_ok=True)
    urls = [
        'https://github.com/xinntao/facexlib/releases/download/v0.1.0/detection_Resnet50_Final.pth',
        'https://github.com/TencentARC/GFPGAN/releases/download/v1.3.0/GFPGANv1.4.pth',
        'https://github.com/xinntao/facexlib/releases/download/v0.2.2/parsing_parsenet.pth',
        'https://github.com/xinntao/Real-ESRGAN/releases/download/v0.2.1/RealESRGAN_x2plus.pth',
        'https://github.com/TencentARC/GFPGAN/releases/download/v1.3.4/RestoreFormer.pth',
        
    ]
    for url in urls:
        filename = url.split('/')[-1]
        model_path = os.path.join(BASE_DIR, filename)
        if os.path.exists(model_path):
            report(f'skipping {filename} model download. File exists')
            continue
        report(f'downloading the model {filename} Please wait...')
        urllib.request.urlretrieve(url, f'{model_path}.tmp', show_progress)
        shutil.move(f'{model_path}.tmp', model_path)
        progress(0, 100, {})


@contextmanager
def enter_gfgan_model_dir():
    # gfpgan only downloads the models at the working directory
    current_dir = os.getcwd()
    try:
        os.chdir(UPSCALERS_DIR)
        yield
    finally:
        os.chdir(current_dir)


def param_or_setting(key, default, args):
    return  args.get(key, get_setting(key, default))


def _gfpgan_upscale(image, scale, restore_bg, args):
    gfpgan_dwonload_model()

    report("restoring faces in the image. Please wait")
    progress(0, 100, {})
    open_cv_image = cv2.cvtColor(numpy.array(image), cv2.COLOR_RGB2BGR)
    with enter_gfgan_model_dir():
        bg_upsampler = None
        if restore_bg:
            from basicsr.archs.rrdbnet_arch import RRDBNet
            from realesrgan import RealESRGANer
            model = RRDBNet(num_in_ch=3, num_out_ch=3, num_feat=64, num_block=23, num_grow_ch=32, scale=2)
            bg_upsampler = RealESRGANer(
                scale=2,
                model_path=os.path.join(BASE_DIR, 'RealESRGAN_x2plus.pth'),
                model=model,
                tile=param_or_setting('gfpgan.tile', 400, args),  
                tile_pad=param_or_setting('gfpgan.tile_pad', 10, args),
                pre_pad=param_or_setting('gfpgan.pre_pad', 0, args),
                half=True)  # need to set False in CPU mode
            
        arch = param_or_setting('gfpgan.arch', 'clean', args)
        
        if arch == 'RestoreFormer':
            model_path = os.path.join(BASE_DIR, 'RestoreFormer.pth')
            channel_multiplier = 2
        else:
            channel_multiplier = 2
            model_path = MODEL_PATH

        restorer = GFPGANer(
            model_path=model_path,
            upscale=scale,
            arch=arch,
            channel_multiplier=channel_multiplier,
            bg_upsampler=bg_upsampler
        )
        cropped_faces, restored_faces, restored_img = restorer.enhance(
                open_cv_image,
                has_aligned=param_or_setting('gfpgan.has_aligned', False, args),
                only_center_face=param_or_setting('gfpgan.only_center_face', False, args),
                paste_back=param_or_setting('gfpgan.paste_back', True, args),
                weight=param_or_setting('gfpgan.weight', 0.5, args) 
        )
        image = Image.fromarray(cv2.cvtColor(restored_img, cv2.COLOR_BGR2RGB))

    del restorer
    del cropped_faces
    del restored_faces
    del restored_img

    gc.collect()

    return image


def _gfpgan_upscale_check_mem(image, scale, restore_bg, args):
     with models_memory_checker():
        return _gfpgan_upscale(image, scale, restore_bg, args)


def gfpgan_restore_faces(image):
    return _gfpgan_upscale_check_mem(image, 1.0, False, {})


def gfpgan_upscale(image, scale, args):
    return pil_as_dict(_gfpgan_upscale_check_mem(pil_from_dict(image), scale, True, args))