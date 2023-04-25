import os
import subprocess
import urllib.request
import sys
import shutil

PROFILE_NAME = 'MingwDiffusionExpert'

def root_dir():
    return os.path.join(os.path.abspath(os.path.dirname(__file__)), '..')


def build_dir():
    result = os.path.join(root_dir(), 'build')
    os.makedirs(result, exist_ok=True)
    return result

def build_dir_fltk(btype):
    result = os.path.join(root_dir(), 'build', 'fltk', btype)
    os.makedirs(result, exist_ok=True)
    return result

def temp_dir():
    result = os.path.join(root_dir(), 'temp')
    os.makedirs(result, exist_ok=True)
    return result


def mingw_bin_dir():
    return os.path.join(build_dir(), 'mingw64', 'bin')

def deeps_dir():
    return os.path.join(root_dir(), 'dependencies')

def fltk_dir():
    return os.path.join(deeps_dir(), 'fltk')


def download_toolset():
    url = "https://github.com/niXman/mingw-builds-binaries/releases/download/12.2.0-rt_v10-rev2/x86_64-12.2.0-release-posix-seh-msvcrt-rt_v10-rev2.7z"
    filepath = os.path.join(temp_dir(), "mingw.7z")
    if not os.path.exists(filepath):
        print('Downloading Mingw')
        urllib.request.urlretrieve(url, filepath)
    if not os.path.exists(mingw_bin_dir()):
        print('Extracting Mingw')
        subprocess.check_call(['7z.exe', 'x', filepath], cwd=build_dir())

def python_dir():
    result = os.path.join(root_dir(), 'dexpert', 'bin')
    if not os.path.exists(result):
        os.makedirs(result, exist_ok=True)
    return result

def python_binary():
    return os.path.join(python_dir(), 'python310.dll')

def download_python():
    url = 'https://www.python.org/ftp/python/3.10.11/python-3.10.11-embed-amd64.zip'
    filepath = os.path.join(temp_dir(), "python3.10.zip")
    if not os.path.exists(filepath):
        print('Downloading embedded Python')
        urllib.request.urlretrieve(url, filepath)
    if not os.path.exists(python_binary()):
        print('Extracting Python')
        subprocess.check_call(['7z.exe', 'x', filepath], cwd=python_dir())


def recreate_profile():
    try:
        subprocess.check_call(['conan', 'profile', 'new', PROFILE_NAME])
    except Exception:
        pass

def configure_profile():
    recreate_profile()
    settings = get_settings()
    for l in settings:
        subprocess.check_call([
            'conan', 'profile', 'update', l, PROFILE_NAME
        ])


def ensure_have_tool(name, url, args):
    try:
        subprocess.check_call(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except FileNotFoundError:
        print(f'The tool {name} was not found.\nInstall it first! see: {url}\nAdd the path to the tool on the PATH environment variable.')
        sys.exit(1)
    except:
        pass

def ensure_have_7zip():
    ensure_have_tool('7-Zip', 'https://www.7-zip.org/', ['7z.exe'])


def ensure_have_cmake():
    ensure_have_tool('CMake', 'https://cmake.org/download/', ['cmake', '--version'])

def apply_patches():
    try:
        nullfile = open(os.devnull, 'w')
        subprocess.check_call([
            'git', 'apply', os.path.join(root_dir(), 'patches', 'fktl-git.patch')
        ], cwd=fltk_dir(), stdout=nullfile, stderr=nullfile)
    except Exception as e:
        print(str(e))

def get_build_type(args):
    if '--debug' in args:
        return 'Debug'
    else:
        return 'Release'

def cmake_args(args):
    mingw_dir = mingw_bin_dir()
    mingw_dir = mingw_dir.replace('\\', '/')
    return [
        f'-DCMAKE_BUILD_TYPE={get_build_type(args)}',
        f'-DCMAKE_MAKE_PROGRAM={mingw_dir}/mingw32-make.exe',
        f'-DCMAKE_C_COMPILER={mingw_dir}/x86_64-w64-mingw32-gcc.exe',
        f'-DCMAKE_CXX_COMPILER={mingw_dir}/x86_64-w64-mingw32-g++.exe',
        f'-DCMAKE_RC_COMPILER={mingw_dir}/windres.exe',
        f'-DCMAKE_C_RANLIB={mingw_dir}/ranlib.exe',
        f'-DCMAKE_CXX_RANLIB={mingw_dir}/ranlib.exe',
    ]


def build_fltk(args):
    build_dir = build_dir_fltk(get_build_type(args))
    subprocess.check_call(
        ['cmake'] + cmake_args(args) + [os.path.join(root_dir(), 'dependencies'), '-G', 'MinGW Makefiles'], 
        cwd=build_dir
    )
    subprocess.check_call([
        'cmake', '--build', '.', '-j12'
    ], cwd=build_dir)
    subprocess.check_call([
        'cmake', '--install', '.', '--prefix', os.path.join(build_dir, '..')
    ] , cwd=build_dir)

def build_diffusion_expert(args):
    subprocess.check_call(['cmake'] + cmake_args(args) + ['..', '-G', 'MinGW Makefiles'], cwd=build_dir())
    subprocess.check_call(['cmake', '--build', '.', '-j12'], cwd=build_dir())
    subprocess.check_call([
        'cmake', '--install', '.', '--prefix', os.path.join(build_dir(), 'dexpert')
    ] , cwd=build_dir())


def build(args):
    apply_patches()
    if 'clean-first' in args:
        cacheFiles = [
            os.path.join(build_dir(), 'CMakeCache.txt'),
            os.path.join(build_dir_fltk(), 'CMakeCache.txt')
        ]
        filesDirs = [
            os.path.join(build_dir(), 'CMakeFiles'),
            os.path.join(build_dir_fltk(), 'CMakeFiles')
        ]
        for cacheFile in cacheFiles:
            if os.path.exists(cacheFile):
                os.unlink(cacheFile)
        for filesDir in filesDirs:
            if os.path.exists(filesDir):
                shutil.rmtree(filesDir)

    build_fltk(args)
    build_diffusion_expert(args)

def publish_binaries():
    source_dir = os.path.join(build_dir(), 'bin')
    files = [
        os.path.join(source_dir, 'diffusion-exp-win.exe'),
        os.path.join(source_dir, 'diffusion-exp.exe'),
    ]
    for f in files:
        shutil.copyfile(f, os.path.join(python_dir(), os.path.basename(f)))

def main(args):
    ensure_have_7zip()
    ensure_have_cmake()
    download_toolset()
    download_python()
    build(args)
    publish_binaries()

if __name__ == '__main__':
    main(sys.argv)