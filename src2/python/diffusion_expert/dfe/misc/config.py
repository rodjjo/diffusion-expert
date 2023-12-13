import os
import json
import sys

ROOT_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    '..', '..', '..'
))


BASE_DIR = os.path.normpath(os.path.join(os.path.dirname(sys.executable), '..'))
CONFIG_DIR = os.path.join(BASE_DIR, 'python_stuff','configurations')
MODELS_DIR = os.path.join(BASE_DIR, 'models', 'stable-diffusion')
CACHE_DIR = os.path.join(BASE_DIR, 'models', 'cache')
VAES_DIR = os.path.join(BASE_DIR, 'models', 'vae')
PREPROC_DIR = os.path.join(BASE_DIR, 'models', 'pre-processors')
TEXT_MODEL_DIR = os.path.join(BASE_DIR, 'models', 'text')
DEEPBOORU_DIR = os.path.join(BASE_DIR, 'models', 'deepbooru')
EMBEDDING_DIR = os.path.join(BASE_DIR, 'models', 'embeddings')
LORA_DIR = os.path.join(BASE_DIR, 'models', 'lora')
CONFIG_PATH = os.path.join(ROOT_DIR, 'config', 'config.json')

def create_dirs():
    try:
        os.makedirs(MODELS_DIR, exist_ok=True)
        os.makedirs(VAES_DIR, exist_ok=True)
        os.makedirs(CACHE_DIR, exist_ok=True)
        os.makedirs(DEEPBOORU_DIR, exist_ok=True)
        os.makedirs(EMBEDDING_DIR, exist_ok=True)
        os.makedirs(LORA_DIR, exist_ok=True)
        os.makedirs(TEXT_MODEL_DIR, exist_ok=True)
        os.makedirs(PREPROC_DIR, exist_ok=True)
    except Exception as e:
        print(f'Error creating directories: {str(e)}')

LAST_SAVED_CONFIG = {}

def load_settings() -> dict:
    global LAST_SAVED_CONFIG
    if not os.path.exists(CONFIG_PATH):
        return {}
    print(f"loading the configuration {CONFIG_PATH}...")
    with open(CONFIG_PATH, 'r') as fp:
        LAST_SAVED_CONFIG = json.load(fp)
        return LAST_SAVED_CONFIG


def store_settings(settings: dict):
    global LAST_SAVED_CONFIG
    if settings == LAST_SAVED_CONFIG:
        return
    print(f"saving the configuration {CONFIG_PATH}...")
    dir = os.path.dirname(CONFIG_PATH)
    if not os.path.exists(dir):
        os.makedirs(dir, exist_ok=True)
    with open(CONFIG_PATH, 'w') as fp:
        json.dump(settings, fp, indent=2)
        LAST_SAVED_CONFIG = settings

def current_config():
    global LAST_SAVED_CONFIG
    if not LAST_SAVED_CONFIG:
        load_settings()
    return LAST_SAVED_CONFIG

def get_config():
    if not LAST_SAVED_CONFIG:
        load_settings()
    return LAST_SAVED_CONFIG

def get_additional_model_dir():
    config = current_config()
    return config.get('directories', {}).get('add_model_dir')


def get_lora_model_dir():
    config = current_config()
    return config.get('directories', {}).get('add_lora_dir')


def get_model_path(model: str):
    model_path = os.path.join(MODELS_DIR, model)
    if not os.path.exists(model_path):
        add_dir = get_additional_model_dir()
        if add_dir:
            model_path = os.path.join(get_additional_model_dir(), model)
    return model_path

def get_lora_location(lora: str) -> str:
    for d in (LORA_DIR, get_lora_model_dir()):
        for e in ('.safetensors', '.ckpt'):
            filepath = os.path.join(d, f'{lora}{e}')
            if os.path.exists(filepath):
                return filepath
    return None


create_dirs()