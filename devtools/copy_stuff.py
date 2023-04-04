import os
import shutil

CURRENT_DIR = os.path.abspath(os.path.dirname(__file__))

def copy_files():
    root_dir =  os.path.join(CURRENT_DIR, '..')
    target_root = os.path.join(root_dir, 'dexpert') 
    target_dir = os.path.join(target_root, 'python_stuff')
    source_dir = os.path.join(root_dir, 'python_stuff')
    config_dir = os.path.join(root_dir, 'python_configs')
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    shutil.copytree(source_dir, target_dir)
    shutil.copyfile(os.path.join(config_dir, 'python310._pth'), os.path.join(target_root, 'bin', 'python310._pth'))
    
    os.makedirs(os.path.join(target_dir, '..', 'bin', 'Lib', 'site-packages'), exist_ok=True)

if __name__ == '__main__':
    copy_files()
