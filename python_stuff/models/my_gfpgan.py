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


from exceptions.exceptions import CancelException

from dexpert import progress, progress_canceled, progress_title

UPSCALERS_DIR =  os.path.join(os.path.dirname(sys.executable), '..', 'models', 'upscalers')
BASE_DIR = os.path.join(UPSCALERS_DIR, 'gfpgan', 'weights')

# make sure it downloads the file inside the models_Dir
MODEL_FILE_NAME = 'GFPGANv1.3.pth'
MODEL_PATH = os.path.join(BASE_DIR, MODEL_FILE_NAME)

def report(message):
    progress_title(f'[GFPGAN Upscaler] - {message}')

def show_progress(block_num, block_size, total_size):
    progress(block_num * block_size, total_size, None)
    if progress_canceled():
        raise CancelException()

def gfpgan_dwonload_model():
    os.makedirs(BASE_DIR, exist_ok=True)
    urls = [
        'https://github.com/xinntao/facexlib/releases/download/v0.1.0/detection_Resnet50_Final.pth',
        'https://github.com/TencentARC/GFPGAN/releases/download/v1.3.0/GFPGANv1.3.pth',
        'https://github.com/xinntao/facexlib/releases/download/v0.2.2/parsing_parsenet.pth'
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


def gfpgan_restore_faces(image):
    report("restoring faces in the image. Please wait")
    progress(0, 100, {})
    open_cv_image = cv2.cvtColor(numpy.array(image), cv2.COLOR_RGB2BGR)
    with enter_gfgan_model_dir():
        restorer = GFPGANer(
            model_path=MODEL_PATH,
            upscale=1,
            arch=get_setting('gfpgan.arch', 'clean'),
            channel_multiplier=get_setting('gfpgan.channel_multiplier', 2),
            bg_upsampler=None
        )
        cropped_faces, restored_faces, restored_img = restorer.enhance(
                open_cv_image,
                has_aligned=get_setting('gfpgan.has_aligned', False),
                only_center_face=get_setting('gfpgan.only_center_face', False),
                paste_back=get_setting('gfpgan.paste_back', True),
                weight=get_setting('gfpgan.weight', 0.5) 
        )
        image = Image.fromarray(cv2.cvtColor(restored_img, cv2.COLOR_BGR2RGB))

    del restorer
    del cropped_faces
    del restored_faces
    del restored_img

    gc.collect()

    return image
