import os
import sys
import subprocess
import urllib.request

BASE_DIR = os.path.normpath(os.path.abspath(os.path.dirname(__file__)))
PYTHON_DIR = os.path.normpath(os.path.abspath(os.path.dirname(sys.executable)))
DEPS_DIR = os.path.join(PYTHON_DIR, '..', 'python_deps')
LIBS_DIR = os.path.join(PYTHON_DIR, 'Lib', 'site-packages')

requirements = [
    'Pillow==10.0.0',
    'OmegaConf==2.3.0',
    'accelerate==0.19.0',
    'numpy==1.23.5',
    'diffusers==0.23.0',
    'transformers==4.30.0',
    'safetensors==0.3.1',
    'invisible-watermark==0.1.5',
    'GitPython==3.1.37',
    'pytorch_lightning==2.0.2',
    'torchdiffeq==0.2.3',
    'tdqm',
    'xformers==0.0.19',
    'opencv-contrib-python==4.7.0.72',
    'controlnet_aux==0.0.3',
    'gfpgan==1.3.8',
    'realesrgan==0.3.0',
    'pynvml==11.5.0',
    'rembg==2.0.52',
    'torch==2.0.0+cu118',
    'torchvision==0.15.1+cu118',
]

def missing_dependencies():
    dep_names = [i.split('=', maxsplit=1)[0] for i in requirements]
    # dep_names += [i.split('=', maxsplit=1)[0] for i in requirements_torch]
    deps_have = os.listdir(LIBS_DIR)
    deps_have = set([
        d.split('-', maxsplit=1)[0].lower() for d in deps_have if '-' in d])
    for d in dep_names:
        if d.lower().replace('-', '_') not in deps_have:
            return True
    return False

def install_requirements():
    import pip
    local_deps = [
        'python-future',
        'filterpy'
    ]
    for d in local_deps:
        if os.path.exists(os.path.join(LIBS_DIR, d.replace('-', '_'))):
            continue
        subprocess.check_call([sys.executable, '-m', 'pip', 'install', os.path.join(DEPS_DIR, d)])
    subprocess.check_call([sys.executable, '-m', 'pip', 'install'] + requirements + [
          '--index-url', 'https://download.pytorch.org/whl/cu118',
          '--extra-index-url', 'https://pypi.python.org/simple'
    ])


def install_pip():
    url = 'https://bootstrap.pypa.io/get-pip.py'
    filepath = os.path.join(BASE_DIR, 'get-pip.py')
    print('Downloading get-pip.py')
    urllib.request.urlretrieve(url, filepath)
    subprocess.check_call([sys.executable, filepath])
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pip', '--upgrade'])

def have_pip():
    try:
        import pip
        return True
    except ImportError:
        return False
    

def _install_dependencies():
    if missing_dependencies():
        install_requirements()


def install():
    if not have_pip():
        install_pip()
    subprocess.check_call([
        sys.executable, __file__
    ])


if __name__ == '__main__':
    _install_dependencies()
