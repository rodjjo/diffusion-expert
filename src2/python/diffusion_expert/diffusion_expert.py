import traceback

from dfe.images.routines import open_image, save_image  # noqa
from dfe.misc.config import load_settings, store_settings # noqa
from dfe.images.background import remove_background # noqa

def capture_exception(f):
    def decorated(*args, **kwargs):
        try:
            return f(*args, **kwargs)
        except Exception as e:
            with open("exception.txt", "w") as fp:
                fp.write("An exception happened: {}\n".format(str(e)))
                fp.write(traceback.format_exc())
                fp.flush()
            raise
    return decorated

@capture_exception
def generate(params: dict): 
    from dfe.pipelines.generator import generate as real_generator # noqa
    return real_generator(params)
    
@capture_exception
def list_models():
    from dfe.models.model_list import list_models as real_listmodels # noqa
    return real_listmodels()

def main():
    pass

if __name__ == '__main__':
    main()