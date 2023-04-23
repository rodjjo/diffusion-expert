import cv2
import numpy as np
from PIL import Image
import PIL.ImageOps
import gc
from controlnet_aux import HEDdetector, OpenposeDetector
from transformers import pipeline

from utils.images import pil_as_dict
from dexpert import progress, progress_title

def report(message):
    progress_title(f'[Image pre-processor] - {message}')


from models.paths import CACHE_DIR


def image_to_canny(input_image) -> dict:
    def do_it():
        low_threshold = 100
        high_threshold = 200
        image = cv2.Canny(np.array(input_image), low_threshold, high_threshold)
        progress(2, 3, None)
        image = image[:, :, None]
        image = np.concatenate([image, image, image], axis=2)
        image = PIL.ImageOps.invert(Image.fromarray(image))
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result 


def image_to_pose(input_image) -> dict:
    def do_it():
        openpose = OpenposeDetector.from_pretrained('lllyasviel/ControlNet', cache_dir=CACHE_DIR)
        progress(2, 3, None)
        image = openpose(input_image)
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result


def image_to_scribble(input_image) -> dict:
    def do_it():
        hed = HEDdetector.from_pretrained('lllyasviel/ControlNet', cache_dir=CACHE_DIR)
        progress(2, 3, None)
        image = PIL.ImageOps.invert(hed(input_image, scribble=True))
        gc.collect()
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result

def image_to_deepth(input_image) -> dict:
    def do_it():
        depth_estimator = pipeline('depth-estimation')
        progress(2, 3, None)
        image = depth_estimator(input_image)['depth']
        image = np.array(image)
        image = image[:, :, None]
        image = np.concatenate([image, image, image], axis=2)
        image = Image.fromarray(image)
        gc.collect()
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result


def pre_process_image(mode: str, data: dict) -> dict:
    progress(1, 3, None)
    with Image.frombytes(data['mode'], (data['width'], data['height']), data['data']) as im:
        if mode == 'canny':
            report("extracting canny edges")
            return image_to_canny(im)
        elif mode == 'scribble':
            report("converting to scribbles")
            return image_to_scribble(im)
        elif mode == 'pose':
            report("extracting pose")
            return image_to_pose(im)
        elif mode == 'deepth':
            report("extracting deepth")
            return image_to_deepth(im)
    return None