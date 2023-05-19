import cv2
import numpy as np
from PIL import Image
import PIL.ImageOps
import gc
from controlnet_aux import HEDdetector, OpenposeDetector
from transformers import pipeline

from utils.images import pil_as_dict, pil_from_dict
from images.line_art import LineartDetector
from images.manga_line import MangaLineExtration
from dexpert import progress, progress_title

def report(message):
    progress_title(f'[Image pre-processor] - {message}')


from models.paths import CACHE_DIR

def HWC3(x):
    assert x.dtype == np.uint8
    if x.ndim == 2:
        x = x[:, :, None]
    assert x.ndim == 3
    H, W, C = x.shape
    assert C == 1 or C == 3 or C == 4
    if C == 3:
        return x
    if C == 1:
        return np.concatenate([x, x, x], axis=2)
    if C == 4:
        color = x[:, :, 0:3].astype(np.float32)
        alpha = x[:, :, 3:4].astype(np.float32) / 255.0
        y = color * alpha + 255.0 * (1.0 - alpha)
        y = y.clip(0, 255).astype(np.uint8)
        return y


def pad64(x):
    return int(np.ceil(float(x) / 64.0) * 64 - x)


def safer_memory(x):
    # Fix many MAC/AMD problems
    return np.ascontiguousarray(x.copy()).copy()


def resize_image_with_pad(input_image, resolution):
    img = HWC3(input_image)
    H_raw, W_raw, _ = img.shape
    k = float(resolution) / float(min(H_raw, W_raw))
    interpolation = cv2.INTER_CUBIC if k > 1 else cv2.INTER_AREA
    H_target = int(np.round(float(H_raw) * k))
    W_target = int(np.round(float(W_raw) * k))
    img = cv2.resize(img, (W_target, H_target), interpolation=interpolation)
    H_pad, W_pad = pad64(H_target), pad64(W_target)
    img_padded = np.pad(img, [[0, H_pad], [0, W_pad], [0, 0]], mode='edge')

    def remove_pad(x):
        return safer_memory(x[:H_target, :W_target])

    return safer_memory(img_padded), remove_pad



def image_to_canny(input_image) -> dict:
    def do_it():
        low_threshold = 100
        high_threshold = 200
        image = cv2.Canny(np.array(input_image), low_threshold, high_threshold)
        progress(2, 3, {})
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
        progress(2, 3, {})
        image = openpose(input_image)
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result


def image_to_scribble(input_image) -> dict:
    def do_it():
        hed = HEDdetector.from_pretrained('lllyasviel/ControlNet', cache_dir=CACHE_DIR)
        progress(2, 3, {})
        image = PIL.ImageOps.invert(hed(input_image, scribble=True))
        gc.collect()
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result

def image_to_deepth(input_image) -> dict:
    def do_it():
        depth_estimator = pipeline('depth-estimation')
        progress(2, 3, {})
        image = depth_estimator(input_image)['depth']
        image = np.array(image)
        image = image[:, :, None]
        image = np.concatenate([image, image, image], axis=2)
        image = Image.fromarray(image)
        return pil_as_dict(image)

    result = do_it()
    gc.collect()
    return result


def image_to_lineart(input_image) -> dict:
    def do_it():
        # la = LineartDetector(LineartDetector.model_default)
        la = LineartDetector(LineartDetector.model_coarse)
        image, remove_pad = resize_image_with_pad(np.array(input_image), 512)
        image = la(image)
        image = image[:, :, None]
        image = np.concatenate([image, image, image], axis=2)
        image[image > 96] = 255
        # image[image < 32] = 0
        image = remove_pad(image)
        image = Image.fromarray(image)
        return pil_as_dict(image)
    result = do_it()
    gc.collect()
    return result


def image_to_mangaline(input_image) -> dict:
    def do_it():
        la = MangaLineExtration()
        image, remove_pad = resize_image_with_pad(np.array(input_image), 512)
        image = la(image)
        image = image[:, :, None]
        image = np.concatenate([image, image, image], axis=2)
        image = remove_pad(image)
        image = Image.fromarray(image)
        return pil_as_dict(image)
    result = do_it()
    gc.collect()
    return result


def pre_process_image(mode: str, data: dict) -> dict:
    progress(1, 3, {})
    with pil_from_dict(data) as im:
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
        elif mode == 'lineart':
            report("extracting lineart")
            return image_to_lineart(im)
        elif mode == 'mangaline':
            report("extracting lineart")
            return image_to_mangaline(im)
    
    return {}