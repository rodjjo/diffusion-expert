from PIL import Image
from dfe.images.routines import pil_as_dict, pil_from_dict


def process(img):
    from rembg import remove, new_session
    return remove(
        img,
        session=new_session('u2net_human_seg'),
        only_mask=True,
        alpha_matting=True,
        alpha_matting_erode_size=15
    )


def add_background(img, rgb_tuple):
    background_image = Image.new('RGB', img.size, rgb_tuple)
    background_image.paste(img, (0, 0), img)
    return background_image


def merge_images(img_list):
    result = Image.new('RGBA', img_list[0].size, (0, 0, 0, 0))
    for img in img_list:
        result.paste(img, (0, 0), mask=img)
        result.paste(result, (0, 0), mask=result.split()[3])
    return result


def remove_background(image, args):
    mask = process(pil_from_dict(image))
    # removed = add_background(removed, (255, 255, 255))
    result = Image.new('RGB', mask.size, (0, 0, 0))
    white = Image.new('RGB', mask.size, (255, 255, 255))
    result.paste(white, (0, 0), mask=mask)
    return pil_as_dict(result)
