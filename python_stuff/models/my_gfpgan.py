import os
import sys
import urllib
import shutil
from contextlib import contextmanager

import numpy
import cv2

from gfpgan import utils, GFPGANer  
from PIL import Image

from exceptions.exceptions import CancelException

from dexpert import progress, progress_canceled, progress_title

BASE_DIR = os.path.join(os.path.dirname(sys.executable), '..', 'models', 'gfpgan')

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
    
    if os.path.exists(MODEL_PATH):
        report("skipping gfpgan model download. File exists")
        return
    os.makedirs(BASE_DIR, exist_ok=True)
    report("downloading the model GFPGANv1.3.pth. Please wait...")
    url = 'https://github.com/TencentARC/GFPGAN/releases/download/v1.3.0/GFPGANv1.3.pth'
    urllib.request.urlretrieve(url, f'{MODEL_PATH}.tmp', show_progress)
    shutil.move(f'{MODEL_PATH}.tmp', MODEL_PATH)
    progress(0, 100, None)



@contextmanager
def enter_gfgan_model_dir():
    # gfpgan only downloads the models at the working directory
    current_dir = os.getcwd()
    try:
        os.chdir(BASE_DIR)
        yield
    finally:
        os.chdir(current_dir)


def gfpgan_restore_faces(image):
    report("restoring faces in the image. Please wait")
    progress(0, 100, None)
    open_cv_image = cv2.cvtColor(numpy.array(image), cv2.COLOR_RGB2BGR)
    with enter_gfgan_model_dir():
        restorer = GFPGANer(
            model_path=MODEL_PATH,
            upscale=1,
            arch='clean',
            channel_multiplier=2,
            bg_upsampler=None
        )
        cropped_faces, restored_faces, restored_img = restorer.enhance(
                open_cv_image,
                has_aligned=False,
                only_center_face=False,
                paste_back=True,
                weight=0.5)
        image = Image.fromarray(cv2.cvtColor(restored_img, cv2.COLOR_BGR2RGB))
    return image