import os
import urllib
import shutil
from exceptions.exceptions import CancelException

from dexpert import progress, progress_canceled, progress_title


def show_progress(block_num, block_size, total_size):
    progress(block_num * block_size, total_size, {})
    if progress_canceled():
        raise CancelException()


def report(message):
    progress_title(f'[Model downloader] - {message}')



def download_file(url, directory, filename):
    progress(0, 100, {})
    os.makedirs(directory, exist_ok=True)
    model_path = os.path.join(directory, filename)
    if os.path.exists(model_path):
        report(f'skipping {filename} model download. File exists')
        report(f'Processing...')
        return
    report(f'URL: {url}')
    report(f'downloading the model {filename} Please wait...')
    try:
        urllib.request.urlretrieve(url, f'{model_path}.tmp', show_progress)
    except CancelException:
        return
    shutil.move(f'{model_path}.tmp', model_path)
    if model_path.endswith('.zip'):
        report("Extracting the zip contents")
        import zipfile
        with zipfile.ZipFile(model_path, "r") as zip_ref:
            zip_ref.extractall(directory)
        os.remove(model_path)
    report(f'Processing...')
    progress(0, 100, {})