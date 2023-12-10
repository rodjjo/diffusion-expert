import os
import shutil

CURRENT_DIR = os.path.abspath(os.path.dirname(__file__))

def copy_files():
    root_dir =  os.path.join(CURRENT_DIR, '..')
    target_root = os.path.join(root_dir, 'dexpert') 
    target_dir = os.path.join(target_root, 'python_stuff')
    target_dir2 = os.path.join(target_root, 'diffusion_expert')
    target_deps_dir = os.path.join(target_root, 'python_deps')
    source_dir = os.path.join(root_dir, 'python_stuff')
    source_dir2 = os.path.join(root_dir, 'src2', 'python', 'diffusion_expert')
    filterpy_dir = os.path.join(root_dir, 'dependencies', 'filterpy')
    comfyui_dir = os.path.join(root_dir, 'dependencies', 'comfyui')
    python_future_dir = os.path.join(root_dir, 'dependencies', 'python-future')
    config_dir = os.path.join(root_dir, 'python_configs')
    target_future = os.path.join(target_deps_dir, 'python-future')
    target_filterpy = os.path.join(target_deps_dir, 'filterpy')
    target_comfyui = os.path.join(target_deps_dir, 'comfyui')
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    if os.path.exists(target_dir2):
        shutil.rmtree(target_dir2)
    if not os.path.exists(target_filterpy):
        shutil.copytree(filterpy_dir, target_filterpy)
    if not os.path.exists(target_future):
        shutil.copytree(python_future_dir, target_future)
    if not os.path.exists(target_comfyui):
        shutil.copytree(comfyui_dir, target_comfyui)
    shutil.copytree(source_dir, target_dir)
    shutil.copytree(source_dir2, target_dir2)
    os.makedirs(os.path.join(target_root, 'bin'), exist_ok=True)
    shutil.copyfile(os.path.join(config_dir, 'python310._pth'), os.path.join(target_root, 'bin', 'python310._pth'))
    
    os.makedirs(os.path.join(target_dir, '..', 'bin', 'Lib', 'site-packages'), exist_ok=True)


if __name__ == '__main__':
    copy_files()
