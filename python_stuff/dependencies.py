import os
import sys
import subprocess
import urllib.request

PRINT_PREFIX = 'dependencies.installer:'


def install_local_dependencies():
    basedir = os.path.join(os.path.dirname(sys.executable), '..', 'python_deps')
    subprocess.check_call([
        sys.executable, '-m', 'pip', 'install', os.path.join(basedir, 'python-future')
    ])
    subprocess.check_call([
        sys.executable, '-m', 'pip', 'install', os.path.join(basedir, 'filterpy')
    ])


def have_pip():
    try:
        import pip
        return True
    except ImportError:
        return False


def have_dependencies():
    exe_dir = os.path.dirname(sys.executable)
    lib_dir = os.path.join(exe_dir, 'Lib', 'site-packages')
    lib_names = [
        'torch',
        'torchvision',
        'numpy',
        'diffusers',
        'transformers',
        'xformers',
        'cv2',
        'controlnet_aux',
        'omegaconf',
        'gfpgan',
        'realesrgan',
        'pynvml',
        'PIL',
        'accelerate',
        'pytorch_lightning',
        'safetensors',
        'torchdiffeq',
        'cv2',
        'filterpy',
        'future'
    ]
    for l in lib_names:
        if not os.path.exists(os.path.join(lib_dir, l)):
            sys.stderr.write(f'Missing dependency: {l}\n')
            sys.stderr.flush()
            return False
    return True


def download_get_pip():
    url = 'https://bootstrap.pypa.io/get-pip.py'
    filepath = os.path.join(os.path.dirname(__file__), 'get-pip.py')
    print('Downloading get-pip.py')
    urllib.request.urlretrieve(url, filepath)


def _install_dependencies():
    base_dir = os.path.dirname(sys.executable)
    requirements_path = os.path.join(base_dir, '..', 'python_stuff', 'requirements.txt')
    requirements_torch = os.path.join(base_dir, '..', 'python_stuff', 'requirements-torch.txt')
    get_pip = os.path.join(base_dir, '..', 'python_stuff', 'get-pip.py')
    
    path = os.environ['PATH']
    path = path.split(os.pathsep)
    path.append(os.path.join(base_dir, 'Scripts'))
    os.environ['PATH'] = os.pathsep.join(path)

    if not have_pip():
        print(f'{PRINT_PREFIX} It does not have pip. Installing it')
        sys.stdout.flush()
        download_get_pip()
        subprocess.check_call([
            sys.executable, get_pip
        ])
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '--upgrade', 'pip'
        ])
    else:
        print(f'{PRINT_PREFIX} It has pip. Skipping pip installation')    
        sys.stdout.flush()
    if not have_dependencies():
        print(f'{PRINT_PREFIX} It does not have the dependencies. Installing them')
        sys.stdout.flush()
        install_local_dependencies()
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_torch
        ])
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_path
        ])
    else:
        print(f'{PRINT_PREFIX} It already has the dependencies installed.')
        sys.stdout.flush()


def install_dependencies():
    subprocess.check_call([
        sys.executable, __file__
    ])


if __name__ == '__main__':
    _install_dependencies()
