#include <list>
#include <Python.h>

#include "src/python/error_handler.h"

#include "src/config/config.h"
#include "src/python/helpers.h"
#include "src/python/python_module.h"
#include "src/windows/progress_window.h"

namespace dexpert
{
    namespace py
    {
        const char *errorFromPyDict(PyObject *result, const char *def) {
            static char buffer[1024] = {};
            if (PyDict_Check(result)) {
                PyObject *error = PyDict_GetItemString(result, "error");
                if (PyUnicode_Check(error)) {
                    strncpy(buffer, PyUnicode_AsUTF8(error), sizeof(buffer) - 1);
                    buffer[sizeof(buffer) - 1] = 0; // ensure null terminated
                    return buffer;
                }
            }
            return def;
        }

        callback_t install_deps(status_callback_t status_cb)
        {
            return [status_cb]
            {
                PythonModule module("dependencies.installer");
                PyObject *result = NULL;
                bool errors = handle_error();
                const char *msg = NULL;

                if (!module.module())
                {
                    msg = "Could not load Python module dependencies.installer";
                    errors = true;
                }

                if (!errors)
                {
                    PyObject *result = module("install_dependencies", NULL);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "There was a failure installing dependencies";
                    }
                }

                status_cb(errors == false, msg);
            };
        }

        callback_t check_have_deps(status_callback_t status_cb)
        {
            return [status_cb]
            {
                PyObject *result = NULL;
                PythonModule module("dependencies.installer");
                bool errors = handle_error();
                const char *msg = NULL;

                if (!module.module())
                {
                    msg = "Could not load Python module dependencies.installer";
                    errors = true;
                }

                if (!errors)
                {
                    PyObject *result = module("have_dependencies", NULL);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "There was a failure installing dependencies";
                    }
                    else
                    {
                        msg = NULL;
                        errors = (PyObject_IsTrue(result) != 1);
                    }
                }

                status_cb(errors == false, msg);
            };
        }

        callback_t open_image(const char *path, image_callback_t status_cb)
        {
            return [status_cb, path]
            {
                bool errors = false;
                const char *msg = NULL;
                std::shared_ptr<RawImage> img;

                PyObject *result = NULL;
                PythonModule module("images.filesystem");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.filesystem";
                    errors = true;
                }

                if (!errors)
                {
                    result = module("open_image", "s", path);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "There was a failure loading the image";
                    }
                    else
                    {
                        img = rawImageFromPyDict(result);
                        if (!img)
                        {
                            errors = true;
                            msg = errorFromPyDict(result, "open_image did not return a valid dictionary with an image info.");
                        }
                    }
                }

                status_cb(!errors, msg, img);
            };
        }

        callback_t save_image(const char *path, RawImage *image, status_callback_t status_cb)
        {
            return [status_cb, path, image]()
            {
                bool errors = false;
                const char *msg = NULL;

                PyObject *result = NULL;
                PyObject *data = NULL;
                PythonModule module("images.filesystem");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.filesystem";
                    errors = true;
                }

                if (!errors)
                {
                    data = module.guard(PyDict_New());
                    image->toPyDict(data);
                    result = module("save_image", "sO", path, data);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "Fail saving the image";
                    }
                }

                status_cb(!errors, msg);
            };
        }


        callback_t pre_process_image(const char *mode, RawImage *image, image_callback_t status_cb)
        {
            enable_progress_window(false);
            return [status_cb, mode, image]()
            {
                std::shared_ptr<RawImage> img;
                bool errors = false;
                const char *msg = NULL;

                PyObject *result = NULL;
                PyObject *data = NULL;
                PythonModule module("images.pre_process");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.pre_process";
                    errors = true;
                }

                if (!errors)
                {
                    data = module.guard(PyDict_New());
                    image->toPyDict(data);
                    result = module("pre_process_image", "sO", mode, data);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "Fail processing the image";
                    } else {
                        img = rawImageFromPyDict(result);
                        if (!img)
                        {
                            errors = true;
                            msg = errorFromPyDict(result, "pre_process_image did not return a valid dictionary with an image info.");
                        }
                    }
                }
                
                status_cb(!errors, msg, img);
            };
        }

        const void txt2img_config_t::fill_prompt_dict(PyObject *params, PythonModule &module) const {
            PyDict_SetItemString(params, "prompt", module.guard(PyUnicode_FromString(this->prompt)));
            PyDict_SetItemString(params, "negative", module.guard(PyUnicode_FromString(this->negative)));
            PyDict_SetItemString(params, "model", module.guard(PyUnicode_FromWideChar(this->model, -1)));
            PyDict_SetItemString(params, "width", module.guard(PyLong_FromSize_t(this->width)));
            PyDict_SetItemString(params, "height", module.guard(PyLong_FromSize_t(this->height)));
            PyDict_SetItemString(params, "steps", module.guard(PyLong_FromSize_t(this->steps)));
            PyDict_SetItemString(params, "cfg", module.guard(PyFloat_FromDouble(this->cfg)));
            PyDict_SetItemString(params, "seed", module.guard(PyLong_FromLong(this->seed)));
            PyDict_SetItemString(params, "variation", module.guard(PyLong_FromLong(this->variation)));
            PyDict_SetItemString(params, "var_stren", module.guard(PyFloat_FromDouble(this->var_stren)));
            PyDict_SetItemString(params, "restore_faces", module.guard(PyBool_FromLong((int)restore_faces)));
            PyDict_SetItemString(params, "enable_codeformer", module.guard(PyBool_FromLong((int)enable_codeformer)));

            if (!this->controlnets.empty()) {
                PyObject *arr = module.guard(PyList_New(0));
                for (auto it = this->controlnets.begin(); it != this->controlnets.end(); it++) {
                    PyObject *c = module.guard(PyDict_New());
                    PyObject *data = module.guard(PyDict_New());
                    it->image->toPyDict(data);
                    PyDict_SetItemString(c, "mode", module.guard(PyUnicode_FromString(it->mode)));
                    PyDict_SetItemString(c, "image", data);
                    PyDict_SetItemString(c, "strength", module.guard(PyFloat_FromDouble(it->strength)));
                    PyList_Append(arr, c);
                }
                PyDict_SetItemString(params, "controlnets",arr);
            }
        }

        const void img2img_config_t::fill_prompt_dict(PyObject *params, PythonModule &module) const { 
            txt2img_config_t::fill_prompt_dict(params, module);
            PyObject *data = module.guard(PyDict_New());
            this->image->toPyDict(data);
            PyDict_SetItemString(params, "image", data);
            PyDict_SetItemString(params, "strength", module.guard(PyFloat_FromDouble(this->strength)));
            if (this->mask) {
                data = module.guard(PyDict_New());
                this->mask->toPyDict(data);
                PyDict_SetItemString(params, "mask", data);
                PyDict_SetItemString(params, "invert_mask", module.guard(PyBool_FromLong(this->invert_mask ? 1 : 0)));
            }
        }

        callback_t get_diffusion_callback(const char* fn_name, const txt2img_config_t &config, image_callback_t status_cb) {
            return [fn_name, &config, status_cb]
            {
                bool errors = false;
                const char *msg = NULL;
                std::shared_ptr<RawImage> img;

                PyObject *result = NULL;
                PythonModule module("images.diffusion_routines");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.diffusion_routines";
                    errors = true;
                }

                PyObject *params = module.guard(PyDict_New());

                config.fill_prompt_dict(params, module);

                if (!errors)
                {
                    result = module(fn_name, "O", params);
                    errors = handle_error();

                    if (errors)
                    {
                        msg = "There was a failure loading the image";
                    }
                    else
                    {
                        img = rawImageFromPyDict(result);
                        if (!img)
                        {
                            errors = true;
                            if (should_cancel_progress())
                            {
                                msg = "The operation was canceled by the user.";
                            }
                            else
                            {
                                msg = errorFromPyDict(result, "It did not return image.");
                            }
                        }
                    }
                }

                status_cb(!errors, msg, img);
            };
        }

        callback_t txt2_image(const txt2img_config_t& config, image_callback_t status_cb)
        {
            enable_progress_window();
            return get_diffusion_callback("txt2img", config, status_cb);
        }

        callback_t img2_image(const img2img_config_t& config, image_callback_t status_cb) {
            enable_progress_window();
            return get_diffusion_callback("img2img", config, status_cb);
        }

        callback_t list_models(const wchar_t *path, model_callback_t status_cb)
        {
            return [path, status_cb]
            {
                model_list_t models;
                bool errors = false;
                const char *msg = NULL;

                PyObject *result = NULL;
                PyObject *data = NULL;
                PythonModule module("models.models");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module models.models";
                    errors = true;
                }

                if (!errors)
                {
                    result = module("list_models", "u", path);
                    errors = handle_error();
                    if (errors || result == NULL)
                    {
                        msg = "Fail loading model list";
                    }
                    else
                    {
                        size_t size = PySequence_Fast_GET_SIZE(result);
                        PyObject *element = NULL;
                        for (size_t i = 0; i < size; ++i)
                        {
                            element = module.guard(PySequence_Fast_GET_ITEM(result, i));
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

        callback_t configure_stable_diffusion(status_callback_t status_cb)
        {
            return [status_cb]
            {
                model_list_t models;
                bool errors = false;
                const char *msg = NULL;

                PyObject *result = NULL;
                PyObject *data = NULL;
                PythonModule module("utils.settings");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module utils.settings";
                    errors = true;
                }
                if (!errors)
                {
                    auto &c = getConfig();
                    PyObject *params = module.newDict();
                    module.dictSetString(params, "scheduler", c.getScheduler());
                    module.dictSetBool(params, "nsfw_filter", c.getSafeFilter() ? 1 : 0);
                    module.dictSetString(params, "device", c.getUseGPU() ? "cuda" : "cpu");
                    module.dictSetBool(params, "use_float16", c.getUseFloat16());
                    module.dictSetString(params, "gfpgan.arch", c.gfpgan_get_arch());
                    module.dictSetInt(params, "gfpgan.channel_multiplier", c.gfpgan_get_channel_multiplier());
                    module.dictSetBool(params, "gfpgan.has_aligned", c.gfpgan_get_has_aligned());
                    module.dictSetBool(params, "gfpgan.only_center_face", c.gfpgan_get_only_center_face());
                    module.dictSetBool(params, "gfpgan.paste_back", c.gfpgan_get_paste_back());
                    module.dictSetFloat(params, "gfpgan.weight", c.gfpgan_get_weight());

                    result = module("set_user_settings", "O", params);
                    errors = handle_error();
                    if (errors)
                    {
                        msg = "Fail setting the configuration";
                    }
                }
                
                status_cb(!errors, msg);
            };
        }

    } // namespace py
} // namespace
