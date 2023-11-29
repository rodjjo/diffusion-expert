import os
import urllib.request
import subprocess
import sys

import build
import copy_stuff

def main():
    prefix = []
    if '--build' in sys.argv:
         args = []
         if '--debug' in sys.argv:
             args = ['--debug']
         build.main(args)
    if '--copy-stuff' in sys.argv:
        copy_stuff.copy_files()
    if '--gdb' in sys.argv:
        prefix = [
            'build/mingw64/bin/gdb.exe'
        ]
    #subprocess.check_call(prefix + [
    #    'dexpert/bin/diffusion-exp2.exe'
    #])

    subprocess.check_call(prefix + [
        'dexpert/bin/diffusion-exp2.exe'
    ])


if __name__ == '__main__':
    main()