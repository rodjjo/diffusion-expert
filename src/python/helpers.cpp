#include <list>
#include <Python.h>

#include "src/python/error_handler.h"

#include "src/python/helpers.h"


namespace dexpert {
namespace py {

callback_t install_dependencies_helper(status_callback_t status_cb) {
    return [status_cb] {
        PyObject *result = NULL;
        PyObject* module = PyImport_ImportModule("dependencies.installer"); 
        bool errors = handle_error();
        const char* msg = NULL;

        if (!module) {
            msg = "Could not load Python module dependencies.installer";
            errors = true;
        }

        if (!errors) {
            PyObject *result = PyObject_CallMethod( module, "install_dependencies", NULL );
            errors = handle_error();
            if (errors) {
                msg = "There was a failure installing dependencies";
            }
        }

        Py_XDECREF(result);
        Py_XDECREF(module);

        status_cb(errors == false, msg);
    };
}

callback_t open_image(const char* path, image_callback_t status_cb) {
    return [status_cb, path] {
        bool errors = false;
        const char* msg = NULL;
        std::shared_ptr<RawImage> img;

        PyObject *result = NULL;
        PyObject* module = PyImport_ImportModule("images.filesystem"); 

        errors = handle_error();

        if (!module) {
            msg = "Could not load Python module images.filesystem";
            errors = true;
        }

        if (!errors) {
            result = PyObject_CallMethod(module, "open_image", "s", path);
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

        Py_XDECREF(result);
        Py_XDECREF(module);

        status_cb(!errors, msg, img);
    };
}

callback_t save_image(const char* path, std::shared_ptr<RawImage> image, status_callback_t status_cb) {
    return [status_cb, path, image] () {
        bool errors = false;
        const char* msg = NULL;

        PyObject *result = NULL;
        PyObject *data = NULL;
        PyObject* module = PyImport_ImportModule("images.filesystem"); 

        errors = handle_error();

        if (!module) {
            msg = "Could not load Python module images.filesystem";
            errors = true;
        }

        if (!errors) {
            data = PyDict_New();
            image->toPyDict(data);
            result = PyObject_CallMethod(module, "save_image", "sO", path, data);
            errors = handle_error();
            if (errors) {
                msg = "Fail saving the image";
            }
        }

        Py_XDECREF(data);
        Py_XDECREF(result);
        Py_XDECREF(module);

        status_cb(!errors, msg);
    };
}


callback_t txt2_image(txt2img_config_t config, image_callback_t status_cb) {
    return [config, status_cb] {
        bool errors = false;
        const char *msg = NULL;
        std::shared_ptr<RawImage> img;

        PyObject *result = NULL;
        PyObject* module = PyImport_ImportModule("images.txt2img"); 

        errors = handle_error();

        if (!module) {
            msg = "Could not load Python module images.txt2img";
            errors = true;
        }

        PyObject *params = PyDict_New();
        std::list<PyObject *> fields;

        fields.push_back(PyUnicode_FromString(config.prompt));
        PyDict_SetItemString(params, "prompt", *fields.rbegin());
        fields.push_back(PyUnicode_FromString(config.negative));
        PyDict_SetItemString(params, "negative", *fields.rbegin());
        fields.push_back(PyUnicode_FromWideChar(config.model, -1));
        PyDict_SetItemString(params, "model", *fields.rbegin());
        fields.push_back(PyLong_FromSize_t(config.width));
        PyDict_SetItemString(params, "width", *fields.rbegin());
        fields.push_back(PyLong_FromSize_t(config.height));
        PyDict_SetItemString(params, "height", *fields.rbegin());
        fields.push_back(PyLong_FromLong(config.seed));
        PyDict_SetItemString(params, "seed", *fields.rbegin());

        if (!errors) {
            result = PyObject_CallMethod(module, "txt2img", "O", params);
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

        for (auto i = fields.begin(); i != fields.end(); i++) {
            Py_XDECREF(*i);
        }

        Py_XDECREF(params);
        Py_XDECREF(result);
        Py_XDECREF(module);

        status_cb(!errors, msg, img);        
    };
}

}  // namespace py
}  // namespace

