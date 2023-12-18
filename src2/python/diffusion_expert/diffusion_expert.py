import traceback

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
    from dfe.pipelines.generator import generate as real_generator 
    return real_generator(params)
    
@capture_exception
def list_models():
    from dfe.models.model_list import list_models as real_listmodels 
    return real_listmodels()


@capture_exception
def open_image(*args, **kwargs):
    from dfe.images.routines import open_image as open_image_original      
    return open_image_original(*args, **kwargs)


@capture_exception
def save_image(*args, **kwargs):
    from dfe.images.routines import save_image as save_image_original      
    return save_image_original(*args, **kwargs)


@capture_exception
def load_settings(*args, **kwargs):
    from dfe.misc.config import load_settings as load_settings_original
    return load_settings_original(*args, **kwargs)


@capture_exception
def store_settings(*args, **kwargs):
    from dfe.misc.config import store_settings as store_settings_original
    return store_settings_original(*args, **kwargs)


@capture_exception
def remove_background(*args, **kwargs):
    from dfe.images.background import remove_background as remove_background_original
    return remove_background_original(*args, **kwargs)


@capture_exception
def list_schedulers(*args, **kwargs):
    from dfe.models.model_list import list_schedulers as list_schedulers_original
    return list_schedulers_original(*args, **kwargs)


@capture_exception
def list_controlnet(*args, **kwargs):
    from dfe.models.model_list import list_controlnets as list_controlnets_original
    return list_controlnets_original(*args, **kwargs)


def main():
    pass

if __name__ == '__main__':
    main()