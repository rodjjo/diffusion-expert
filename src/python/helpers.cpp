#include <list>
#include <Python.h>

#include "src/python/error_handler.h"

#include "src/python/helpers.h"
#include "src/python/guard.h"


namespace dexpert {
namespace py {

callback_t install_deps(status_callback_t status_cb) {
    return [status_cb] {
        ObjGuard guard("dependencies.installer"); 
        PyObject *result = NULL;
        bool errors = handle_error();
        const char* msg = NULL;

        if (!guard.module()) {
            msg = "Could not load Python module dependencies.installer";
            errors = true;
        }

        if (!errors) {
            PyObject *result = guard(PyObject_CallMethod(guard.module(), "install_dependencies", NULL ));
            errors = handle_error();
            if (errors) {
                msg = "There was a failure installing dependencies";
            }
        }

        status_cb(errors == false, msg);
    };
}

callback_t check_have_deps(status_callback_t status_cb) {
    return [status_cb] {
        PyObject *result = NULL;
        ObjGuard guard("dependencies.installer"); 
        bool errors = handle_error();
        const char* msg = NULL;

        if (!guard.module()) {
            msg = "Could not load Python module dependencies.installer";
            errors = true;
        }

        if (!errors) {
            PyObject *result = guard(PyObject_CallMethod(guard.module(), "have_dependencies", NULL));
            errors = handle_error();
            if (errors) {
                msg = "There was a failure installing dependencies";
            } else {
                msg = NULL;
                errors = (PyObject_IsTrue(result) != 1);
            }
        }

        status_cb(errors == false, msg);
    };
}

callback_t open_image(const char* path, image_callback_t status_cb) {
    return [status_cb, path] {
        bool errors = false;
        const char* msg = NULL;
        std::shared_ptr<RawImage> img;

        PyObject *result = NULL;
        ObjGuard guard("images.filesystem"); 

        errors = handle_error();

        if (!guard.module()) {
            msg = "Could not load Python module images.filesystem";
            errors = true;
        }

        if (!errors) {
            result = guard(PyObject_CallMethod(guard.module(), "open_image", "s", path));
            errors = handle_error();
            if (errors) {
                msg = "There was a failure loading the image";
            } else {
                img = rawImageFromPyDict(result);
                if (!img) {
                    errors = true;
                    msg = "open_image did not return a valid dictionary with an image info.";
                }
            }
        }

        status_cb(!errors, msg, img);
    };
}

callback_t save_image(const char* path, std::shared_ptr<RawImage> image, status_callback_t status_cb) {
    return [status_cb, path, image] () {
        bool errors = false;
        const char* msg = NULL;

        PyObject *result = NULL;
        PyObject *data = NULL;
        ObjGuard guard("images.filesystem"); 

        errors = handle_error();

        if (!guard.module()) {
            msg = "Could not load Python module images.filesystem";
            errors = true;
        }

        if (!errors) {
            data = guard(PyDict_New());
            image->toPyDict(data);
            result = guard(PyObject_CallMethod(guard.module(), "save_image", "sO", path, data));
            errors = handle_error();
            if (errors) {
                msg = "Fail saving the image";
            }
        }

        status_cb(!errors, msg);
    };
}


callback_t txt2_image(txt2img_config_t config, image_callback_t status_cb) {
    return [config, status_cb] {
        bool errors = false;
        const char *msg = NULL;
        std::shared_ptr<RawImage> img;

        PyObject *result = NULL;
        ObjGuard guard("images.txt2img"); 

        errors = handle_error();

        if (!guard.module()) {
            msg = "Could not load Python module images.txt2img";
            errors = true;
        }

        PyObject *params = guard(PyDict_New());

        PyDict_SetItemString(params, "prompt", guard(PyUnicode_FromString(config.prompt)));
        PyDict_SetItemString(params, "negative", guard(PyUnicode_FromString(config.negative)));
        PyDict_SetItemString(params, "model", guard(PyUnicode_FromWideChar(config.model, -1)));
        PyDict_SetItemString(params, "width", guard(PyLong_FromSize_t(config.width)));
        PyDict_SetItemString(params, "height", guard(PyLong_FromSize_t(config.height)));
        PyDict_SetItemString(params, "steps", guard(PyLong_FromSize_t(config.steps)));
        PyDict_SetItemString(params, "cfg", guard(PyFloat_FromDouble(config.cfg)));
        PyDict_SetItemString(params, "seed", guard(PyLong_FromLong(config.seed)));
        PyDict_SetItemString(params, "variation", guard(PyLong_FromLong(config.variation)));
        PyDict_SetItemString(params, "var_step", guard(PyFloat_FromDouble(config.var_step)));

        if (!errors) {
            result = guard(PyObject_CallMethod(guard.module(), "txt2img", "O", params));
            errors = handle_error();
            if (errors) {
                msg = "There was a failure loading the image";
            } else {
                img = rawImageFromPyDict(result);
                if (!img) {
                    errors = true;
                    msg = "open_image did not return a valid dictionary with an image info.";
                }
            }
        }

        status_cb(!errors, msg, img);        
    };
}

callback_t list_models(const wchar_t* path, model_callback_t status_cb) {
    return [path, status_cb] {
        model_list_t models;
        bool errors = false;
        const char* msg = NULL;

        PyObject *result = NULL;
        PyObject *data = NULL;
        ObjGuard guard("models.models"); 

        errors = handle_error();

        if (!guard.module()) {
            msg = "Could not load Python module models.models";
            errors = true;
        }

        if (!errors) {
            result = guard(PyObject_CallMethod(guard.module(), "list_models", "u", path));
            errors = handle_error();
            if (errors || result == NULL) {
                msg = "Fail loading model list";
            } else {
                size_t size = PySequence_Fast_GET_SIZE(result);
                PyObject *element = NULL;
                for (size_t i = 0; i < size; ++i) {
                    element = guard(PySequence_Fast_GET_ITEM(result, i));
                    PyObject *name = PyDict_GetItemString(element, "name");
                    PyObject *size = PyDict_GetItemString(element, "size");
                    PyObject *hash = PyDict_GetItemString(element, "hash");
                    PyObject *path = PyDict_GetItemString(element, "path");
                    model_t model;
                    model.name = PyUnicode_AsUTF8(name);
                    model.hash = PyUnicode_AsUTF8(hash);
                    
                    Py_ssize_t sz = 0;
                    wchar_t *text = PyUnicode_AsWideCharString(path, &sz);
                    model.path = std::wstring(text, sz);
                    PyMem_Free(text);

                    model.size = PyLong_AsSize_t(size);
                    models.push_back(model);
                }
            }
        }

        status_cb(!errors, msg, models);
    };
}

}  // namespace py
}  // namespace

