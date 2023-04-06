import multiprocessing

PRINT_PREFIX = 'dependencies.installer:'


def have_pip():
    try:
        import pip
        return True
    except ImportError:
        return False


def have_dependencies():
    try:
        import torch
        import numpy
        import diffusers
        import transformers
        from omegaconf import OmegaConf
        return True
    except ImportError:
        return False
    

def _install_dependencies():
    import sys
    import subprocess
    import os

    base_dir = os.path.dirname(sys.executable)
    requirements_path = os.path.join(base_dir, '..', 'python_stuff', 'requirements.txt')
    requirements_torch = os.path.join(base_dir, '..', 'python_stuff', 'requirements-torch.txt')
    get_pip = os.path.join(base_dir, '..', 'python_stuff', 'dependencies', 'get-pip.py')
    
    if not have_pip():
        print(f'{PRINT_PREFIX} It does not have pip. Installing it')
        sys.stdout.flush()
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
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_torch
        ])
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_path
        ])
    else:
        print(f'{PRINT_PREFIX} It already has the dependencies installed.')
        sys.stdout.flush()

def simulator():
    from tqdm import tqdm
    import time
    import sys
    for i in tqdm(range(3000), file=sys.stdout, colour="green"):
        time.sleep(0.003)

def install_dependencies():
    proc = multiprocessing.Process(target=_install_dependencies)
    proc.start()
    proc.join()
