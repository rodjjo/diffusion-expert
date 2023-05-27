import os
import sys

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


create_dirs()