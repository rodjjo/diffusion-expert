from PIL import Image
from utils.images import pil_as_dict, pil_from_dict


def open_image(path: str):
    print(f"Open image: {path}")
    with Image.open(path) as im:
        return pil_as_dict(im)


def save_image(path: str, data: dict):
    print(f"Save image: {path}")
    ends = path.lower().endswith
    with pil_from_dict(data, (ends('.jpg') or ends('jpeg'))) as im:
        im.save(path)
