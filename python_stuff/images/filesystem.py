
from PIL import Image


def open_image(path: str):
    print(f"Open image: {path}")
    with Image.open(path) as im:
        data = im.tobytes()
        return {
            'data': im.tobytes(),
            'width': im.width,
            'height': im.height,
            'mode': im.mode,
        }


def save_image(path: str, data: dict):
    print(f"Save image: {path}")
    with Image.frombytes(data['mode'], (data['width'], data['height']), data['data']) as im:
        ends = path.lower().endswith
        if data['mode'] != 'RGB' and (ends('.jpg') or ends('jpeg')):
            with im.convert('RGB') as im2:
                im2.save(path)
                return
        im.save(path)
