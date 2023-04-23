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
                    PyObject *result = module.call("install_dependencies", NULL);
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
                    PyObject *result = module.call("have_dependencies", NULL);
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
                    result = module.call("open_image", "s", path);
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
                PythonModule module("images.filesystem");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.filesystem";
                    errors = true;
                }

                if (!errors)
                {
                    Dict data = module.newDict();
                    image->toPyDict(&data);
                    result = module.call("save_image", "sO", path, data.obj());
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
                PythonModule module("images.pre_process");

                errors = handle_error();

                if (!module.module())
                {
                    msg = "Could not load Python module images.pre_process";
                    errors = true;
                }

                if (!errors)
                {
                    Dict data = module.newDict();
                    image->toPyDict(&data);
                    result = module.call("pre_process_image", "sO", mode, data.obj());
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

        const void txt2img_config_t::fill_prompt_dict(Dict *params, PythonModule &module) const {
            params->setString("prompt", this->prompt);
            params->setString("negative", this->negative);
            params->setWString("model", this->model);
            params->setInt("width", this->width);
            params->setInt("height", this->height);
            params->setInt("steps", this->steps);
            params->setFloat("cfg", this->cfg);
            params->setInt("seed", this->seed);
            params->setInt("variation", this->variation);
            params->setFloat("var_stren", this->var_stren);
            params->setBool("restore_faces", restore_faces);
            params->setBool("enable_codeformer", enable_codeformer);

            if (!this->controlnets.empty()) {
                PyObject *arr = PyList_New(0);
                for (auto it = this->controlnets.begin(); it != this->controlnets.end(); it++) {
                    Dict c = module.newDict();
                    Dict data = module.newDict();
                    
                    it->image->toPyDict(&data);

                    c.setString("mode", it->mode);
                    c.setDict("image", data);
                    c.setFloat("strength", it->strength);

                    PyList_Append(arr, c.obj());
                }
                params->setAny("controlnets", arr);
            }
        }

        const void img2img_config_t::fill_prompt_dict(Dict *params, PythonModule &module) const { 
            txt2img_config_t::fill_prompt_dict(params, module);
            Dict data = module.newDict();
            this->image->toPyDict(&data);
            params->setDict("image", data);
            params->setFloat("strength", this->strength);

            if (this->mask) {
                data = module.newDict();
                this->mask->toPyDict(&data);
                params->setDict("mask", data);
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

                Dict params = module.newDict();

                config.fill_prompt_dict(&params, module);

                if (!errors)
                {
                    result = module.call(fn_name, "O", params.obj());
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
                    result = module.call("list_models", "u", path);
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
                    Dict params = module.newDict();
                    
                    params.setString("scheduler", c.getScheduler());
                    params.setBool("nsfw_filter", c.getSafeFilter() ? 1 : 0);
                    params.setString("device", c.getUseGPU() ? "cuda" : "cpu");
                    params.setBool("use_float16", c.getUseFloat16());
                    params.setString("gfpgan.arch", c.gfpgan_get_arch());
                    params.setInt("gfpgan.channel_multiplier", c.gfpgan_get_channel_multiplier());
                    params.setBool("gfpgan.has_aligned", c.gfpgan_get_has_aligned());
                    params.setBool("gfpgan.only_center_face", c.gfpgan_get_only_center_face());
                    params.setBool("gfpgan.paste_back", c.gfpgan_get_paste_back());
                    params.setFloat("gfpgan.weight", c.gfpgan_get_weight());

                    result = module.call("set_user_settings", "O", params.obj());
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
