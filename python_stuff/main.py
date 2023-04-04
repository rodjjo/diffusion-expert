import sys
import os 
import subprocess

import multiprocessing

EXE_DIR = os.path.dirname(sys.executable)
BASEDIR = os.path.abspath(os.path.dirname(__file__))


def install_pip():
    try:
        pip = __import__('pip')
        print("Have Pip")
        return
    except ImportError:
        print('Installing Pip')
        subprocess.check_call([
            sys.executable,
            os.path.join(BASEDIR, 'dependencies', 'get-pip.py')
        ])
        print('Pip was installed')

def import_deps():
    try:
        import click
        print("have deps")
        return
    except ImportError:
        import pip 
        pip.main(['install', '--upgrade', 'pip'])
        print('Installing requirements')
        pip.main(['install', '-r', os.path.join(BASEDIR, 'requirements.txt')])
        print('Requirements installed')

def say_hi():
    print("Log from subprocess")


if __name__ == '__main__':
    print("goes here!")
    # print(sys.path)
    install_pip()
    
    import_deps()

    p1 = multiprocessing.Process(target=say_hi)
    p1.start()
    p1.join()