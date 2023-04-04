import os
import urllib.request
import subprocess
import sys

import build
import copy_stuff


def python_dir():
    result = os.path.join(build.root_dir(), 'dexpert', 'bin')
    if not os.path.exists(result):
        os.makedirs(result, exist_ok=True)
    return result


def python_binary():
    return os.path.join(python_dir(), 'python310.dll')


def download_python():
    url = 'https://www.python.org/ftp/python/3.10.0/python-3.10.0-embed-amd64.zip'
    filepath = os.path.join(build.temp_dir(), "python3.10.zip")
    if not os.path.exists(filepath):
        print('Downloading embedded Python')
        urllib.request.urlretrieve(url, filepath)
    if not os.path.exists(python_binary()):
        print('Extracting Python')
        subprocess.check_call(['7z.exe', 'x', filepath], cwd=python_dir())


def main():
    if '--build' in sys.argv:
         build.main([])
    if '--copy-stuff' in sys.argv:
        copy_stuff.copy_files()
    download_python()
    subprocess.check_call([
        'dexpert/bin/diffusion-exp.exe'
    ])


if __name__ == '__main__':
    main()