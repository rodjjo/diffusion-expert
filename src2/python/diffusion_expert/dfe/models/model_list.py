import os

from PIL import Image

from safetensors.torch import load_file as load_safetensors
import torch


from dfe.images.routines import pil_as_dict
from dfe.misc.config import MODELS_DIR, get_additional_model_dir, get_textual_inversion_paths, get_lora_paths


def list_models_path(path: str):
    contents = os.listdir(path)
    return [{
            'path': os.path.join(path, m),
            'name': m, 
            'inpaint': 'inpaint' in m,
        } for m in contents if m.endswith('.safetensors')
    ]


def list_models():
    models = list_models_path(
        MODELS_DIR
    )
    add_dir = get_additional_model_dir()
    if add_dir:
        models += list_models_path(
            add_dir
        )
    return models


def list_schedulers():
    return [
        {'name': 'EulerAncestralDiscreteScheduler'},
        {'name': 'DDIMScheduler'},
        {'name': 'PNDMScheduler'},
        {'name': 'UniPCMultistepScheduler'},
        {'name': 'LMSDiscreteScheduler'},
        {'name': 'LCMScheduler'},
    ]


def list_controlnets():
    return [
        { "name": "scribble", "title": "Scribble" },
        { "name": "canny", "title": "Canny Lines" },
        { "name": "pose", "title": "Open Pose" },
        { "name": "deepth", "title": "Deepth Map" },
        { "name": "segmentation", "title": "Segmentation" },
        { "name": "lineart", "title": "Line art" },
        { "name": "mangaline", "title": "Mangaline" },
    ]


def list_textual_inversions():
    files = get_textual_inversion_paths()
    result = []
    for f in files:
        if f[0]:
            data = load_safetensors(f[1], device="cpu")
        else:
            data = torch.load(f[1], map_location="cpu")
        if 'string_to_param' in data:
            if 'name' in data:
                result.append({
                    'name': data['name'],
                    'kind': 'textual_inv',
                    'filename': os.path.basename(f[1]),
                    'path': f[1]
                })
            del data
        elif type(data) == dict and type(next(iter(data.values()))) == torch.Tensor:
            if len(data.keys()) != 1:
                continue
            emb = next(iter(data.values()))
            if len(emb.shape) == 1:
                emb = emb.unsqueeze(0)
            result.append({
                'name': next(iter(data.keys())),
                'kind': 'textual_inv',
                'filename': os.path.basename(f[1]),
                'path': f[1]
            })
    return result


def list_loras():
    result = []
    files = get_lora_paths()
    for f in files:
        name = os.path.basename(f)
        if '.' in name:
            name = name.rsplit('.', maxsplit=1)[0]
        result.append({
            'name': name,
            'kind': 'lora',
            'filename': os.path.basename(f),
            'path': f
        })
    result.sort(key=lambda x: x['name'].lower())
    return result


def make_unique(concept_list: list):
    items = {
        v['name']: v for v in concept_list
    }
    return items.values()


def list_embeddings(lora: bool):
    if lora:
        return make_unique(list_loras())
    return make_unique(list_textual_inversions())

