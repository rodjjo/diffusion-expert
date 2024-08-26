import os
import sys
import subprocess
import json

PRINT_PREFIX = 'dependencies.installer:'

base_dir = os.path.dirname(os.path.abspath(__file__))
requirements_path = os.path.join(base_dir, 'requirements.txt')
requirements_torch = os.path.join(base_dir, 'requirements-torch.txt')    


def fix_dependency_name(name):
    return name.strip().lower().replace('-', '_')


def requirements_contents():
    with open(requirements_path, 'r') as f:
        contents1 = f.read()
    with open(requirements_torch, 'r') as f:
        contents2 = f.read()
    result = (contents1 + '\n' + contents2).split('\n')
    return [
        fix_dependency_name(r.split('=', maxsplit=1)[0]) 
        for r in result if r.strip() and not r.strip().startswith('#') and not r.strip().startswith('-')
    ]


def any_missing_dependency():
    exe_dir = os.path.dirname(sys.executable)
    pip_output = subprocess.check_output([
        sys.executable, '-m', 'pip', 'list', '--format', 'json'
    ]).decode('utf-8')
    installed_libs = set(fix_dependency_name(l['name']) for l in json.loads(pip_output))
    lib_names = requirements_contents()

    for l in lib_names:
        if l not in installed_libs:
            sys.stderr.write(f'Missing dependency: {l}\n')
            sys.stderr.flush()
            return True
    return False

def have_dependencies():
    return not any_missing_dependency()

def _install_dependencies():
    if any_missing_dependency():
        print(f'{PRINT_PREFIX} It does not have the dependencies. Installing them')
        sys.stdout.flush()
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', 'setuptools'
        ])
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_path
        ])
        subprocess.check_call([
            sys.executable, '-m', 'pip', 'install', '-r', requirements_torch
        ])


def install_dependencies():
    subprocess.check_call([
        sys.executable, __file__
    ])
    import importlib
    importlib.invalidate_caches()


if __name__ == '__main__':
    _install_dependencies()