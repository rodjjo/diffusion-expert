import os
import sys

BASE_DIR = os.path.normpath(os.path.join(os.path.dirname(sys.executable), '..'))
CONFIG_DIR = os.path.join(BASE_DIR, 'python_stuff','configurations')
MODELS_DIR = os.path.join(BASE_DIR, 'models', 'stable-diffusion')
CACHE_DIR = os.path.join(BASE_DIR, 'models', 'cache')
VAES_DIR = os.path.join(BASE_DIR, 'models', 'vae')
DEEPBOORU_DIR = os.path.join(BASE_DIR, 'models', 'deepbooru')

def create_dirs():
    try:
        os.makedirs(MODELS_DIR, exist_ok=True)
        os.makedirs(VAES_DIR, exist_ok=True)
        os.makedirs(CACHE_DIR, exist_ok=True)
    except Exception as e:
        print(f'Error creating directories: {str(e)}')


create_dirs()