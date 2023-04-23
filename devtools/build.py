import os
import subprocess
import urllib.request
import sys
import shutil

PROFILE_NAME = 'MingwDiffusionExpert'


def get_settings():
    mingw_dir = mingw_bin_dir()
    mingw_dir = mingw_dir.replace('\\', '/')
    return [
        f'env.CC={mingw_dir}/x86_64-w64-mingw32-gcc.exe',
        f'env.CXX={mingw_dir}/x86_64-w64-mingw32-g++.exe',
        f'env.AR={mingw_dir}/ar.exe',
        f'env.AS={mingw_dir}/as.exe',
        f'env.RANLIB={mingw_dir}/ranlib.exe',
        f'env.STRIP={mingw_dir}/strip.exe',
        f'env.RC={mingw_dir}/windres.exe',
        'env.CONAN_CMAKE_GENERATOR=MinGW Makefiles',
        'env.CMAKE_GENERATOR=MinGW Makefiles',
        'env.CMAKE_BUILD_TYPE=Debug',
        # 'env.CMAKE_CXX_FLAGS=-Wno-error=address',
        f'env.CONAN_CMAKE_FIND_ROOT_PATH={mingw_dir}',
        f'env.CMAKE_MAKE_PROGRAM={mingw_dir}/mingw32-make.exe',
        'settings.arch=x86_64',
        'settings.build_type=Debug',
        'settings.compiler=gcc',
        'settings.compiler.cppstd=gnu23',
        'settings.compiler.exception=seh',
        'settings.compiler.libcxx=libstdc++11',
        'settings.compiler.threads=win32',
        'settings.compiler.version=12.2',
        'settings.os=Windows',
        'settings.os_build=Windows',
        'conf.tools.cmake.cmaketoolchain:generator=MinGW Makefiles',
        # 'conf.tools.build:cxxflags=["-Wno-error=address"]',
        #'conf.tools.build:cflags=["-Wno-error=address"]',
    ]

def root_dir():
    return os.path.join(os.path.abspath(os.path.dirname(__file__)), '..')


def build_dir():
    result = os.path.join(root_dir(), 'build')
    os.makedirs(result, exist_ok=True)
    return result


def temp_dir():
    result = os.path.join(root_dir(), 'temp')
    os.makedirs(result, exist_ok=True)
    return result


def mingw_bin_dir():
    return os.path.join(build_dir(), 'mingw64', 'bin')


def download_toolset():
    url = "https://github.com/niXman/mingw-builds-binaries/releases/download/12.2.0-rt_v10-rev2/x86_64-12.2.0-release-posix-seh-msvcrt-rt_v10-rev2.7z"
    filepath = os.path.join(temp_dir(), "mingw.7z")
    if not os.path.exists(filepath):
        print('Downloading Mingw')
        urllib.request.urlretrieve(url, filepath)
    if not os.path.exists(mingw_bin_dir()):
        print('Extracting Mingw')
        subprocess.check_call(['7z.exe', 'x', filepath], cwd=build_dir())


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


def ensure_have_conan():
    ensure_have_tool('Conan', 'https://conan.io/', ['conan', '--version'])


def ensure_have_7zip():
    ensure_have_tool('7-Zip', 'https://www.7-zip.org/', ['7z.exe'])


def ensure_have_cmake():
    ensure_have_tool('CMake', 'https://cmake.org/download/', ['cmake', '--version'])


def build_envs():
    settings = get_settings()
    PATH = os.environ.get('PATH', '').split(';')
    PATH.append(
        mingw_bin_dir()
    )
    envs = { **os.environ }
    connan_envs = {}
    for line in settings:
        pairs = line.split('=', maxsplit=1)
        connan_envs[pairs[0]] = pairs[1]
    envs['PATH'] = ';'.join(PATH)
    for k in connan_envs.keys():
        if k.startswith('env.'):
            name = k.split('.', maxsplit=1)[1]
            envs[name] = connan_envs[k]
    return envs


def build(args):
    envs = build_envs()
    if '--debug' in args:
        envs["DEBUG_ENABLED"] = "ON"

    additionalArgs = []

    if 'clean-first' in args:
        additionalArgs = ['--clean-first']

    if 'clean-cmake' in args:
        cacheFile = os.path.join(build_dir(), 'CMakeCache.txt')
        filesDir = os.path.join(build_dir(), 'CMakeFiles')
        if os.path.exists(cacheFile):
            os.unlink(cacheFile)
        if os.path.exists(filesDir):
            shutil.rmtree(filesDir)

    subprocess.check_call([
        'conan',
        'install',
        '..',
        f'--profile={PROFILE_NAME}',
        '--output-folder=build',
        '--build=missing',
    ], cwd=build_dir(), env=envs)
    subprocess.check_call([
        'cmake', '..', '-G', 'MinGW Makefiles'
    ], cwd=build_dir(), env=envs)
    subprocess.check_call([
        'cmake', '--build', '.',  '--', '-j', '4'
    ] + additionalArgs, cwd=build_dir(), env=envs)
    subprocess.check_call([
        'cmake', '--install', '.', '--prefix', os.path.join(build_dir(), '..', 'dexpert')
    ] , cwd=build_dir(), env=envs)


def main(args):
    ensure_have_conan()
    ensure_have_7zip()
    ensure_have_cmake()
    download_toolset()
    configure_profile()
    build(args)

if __name__ == '__main__':
    main(sys.argv)