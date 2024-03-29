import os

from safetensors.torch import load_file as load_safetensors
import torch


from dfe.images.routines import pil_as_dict
from dfe.misc.config import MODELS_DIR, MODEL_TYPES, get_additional_model_dir, get_textual_inversion_paths, get_lora_paths


def list_models_path(path: str):
    contents = os.listdir(path)
    result = []
    for f in contents:
        full_path = os.path.join(path, f)
        if os.path.isdir(full_path) and f.lower() in MODEL_TYPES:
            result += list_models_path(full_path)
        elif  f.endswith('.safetensors'):
            result.append({
                'path': os.path.join(path, f),
                'name': f, 
                'inpaint': 'inpaint' in f
            })
    return result


def list_models():
    models = list_models_path(
        MODELS_DIR
    )
    add_dir = get_additional_model_dir()
    if add_dir:
        models += list_models_path(
            add_dir
        )
    models.sort(key=lambda x: x['name'].lower())
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
        { "name": "scribble", "title": "Scribble", "pre_processor": True },
        { "name": "canny", "title": "Canny Lines", "pre_processor": True },
        { "name": "pose", "title": "Open Pose", "pre_processor": True },
        { "name": "deepth", "title": "Deepth Map", "pre_processor": True },
        { "name": "segmentation", "title": "Segmentation", "pre_processor": False },
        { "name": "lineart", "title": "Line art", "pre_processor": True },
        { "name": "mangaline", "title": "Mangaline", "pre_processor": True },
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

