import os

from dfe.misc.config import MODELS_DIR, get_additional_model_dir


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