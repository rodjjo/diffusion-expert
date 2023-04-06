#include <list>
#include <Python.h>

#include "src/python/error_handler.h"

#include "src/python/helpers.h"


namespace dexpert {
namespace py {

class ObjGuard {
  public:
    ObjGuard(const char* module = NULL) {
        if (module) {
            module_ = PyImport_ImportModule(module);
        }
    }

    PyObject *guard(PyObject * v) {
        items_.push_back(v);
        return v;
    }

    PyObject *operator () (PyObject *v) {
        return guard(v);
    }

    ~ObjGuard() {
        Py_XDECREF(module_);
        for (auto it = items_.begin(); it != items_.end(); it++) {
            Py_XDECREF(*it);
        }
    }
    PyObject *module() {
        return module_;
    }
  private:
     PyObject* module_ = NULL;
     std::list<PyObject*> items_;
};

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
        PyDict_SetItemString(params, "seed", guard(PyLong_FromLong(config.seed)));

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

}  // namespace py
}  // namespace

