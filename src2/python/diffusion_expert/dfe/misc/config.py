import os
import json

ROOT_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    '..', '..', '..'
))
MODELS_DIR = os.path.join(ROOT_DIR, 'models')
CONFIG_PATH = os.path.join(ROOT_DIR, 'config', 'config.json')

LAST_SAVED_CONFIG = {}

def load_settings() -> dict:
    if not os.path.exists(CONFIG_PATH):
        return {}
    print(f"loading the configuration {CONFIG_PATH}...")
    with open(CONFIG_PATH, 'r') as fp:
        return json.load(fp)


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
