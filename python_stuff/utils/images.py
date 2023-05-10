from PIL import Image, ImageFilter, ImageOps

def pil_as_dict(pil_image):
    return {
        'data': bytes(pil_image.tobytes()),
        'width': int(pil_image.width),
        'height': int(pil_image.height),
        'mode': str(pil_image.mode),
    }


def rgb_image_512x512():
    return {
        'data': b'\xFF' * (512 * 512 * 3),
        'width': 512,
        'height': 512,
        'mode': 'RGB',
    }


def pil_from_dict(data, convert_rgb=True):
    img = Image.frombytes(data['mode'], (data['width'], data['height']), data['data'])
    if convert_rgb and img.mode.lower() == 'rgba':
        result = Image.new('RGB', (data['width'], data['height']),  (255, 255, 255))
        result.paste(img, (0, 0), img) 
        return result
    return img


def fill_image(image, mask):
    image_mod = Image.new('RGBA', (image.width, image.height))

    image_masked = Image.new('RGBa', (image.width, image.height))
    image_masked.paste(image.convert("RGBA").convert("RGBa"), mask=ImageOps.invert(mask.convert('L')))

    image_masked = image_masked.convert('RGBa')

    for radius, repeats in [(256, 1), (64, 1), (16, 2), (4, 4), (2, 2), (0, 1)]:
        blurred = image_masked.filter(ImageFilter.GaussianBlur(radius)).convert('RGBA')
        for _ in range(repeats):
            image_mod.alpha_composite(blurred)

    return image_mod.convert("RGB")
