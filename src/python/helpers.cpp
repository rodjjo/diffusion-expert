#include <list>
#include <Python.h>

#include "src/config/config.h"
#include "src/python/helpers.h"
#include "src/windows/progress_window.h"
#include "src/python/wrapper.h"

namespace dexpert
{
    namespace py
    {
        const char *getError(const std::runtime_error &e) {
            static std::string error;
            error = e.what();
            return error.c_str();
        }

        const char *errorFromPyDict(py11::dict &result, const char *def)
        {
            static std::string buffer;
            buffer = def;
            if (result.contains("error"))
            {
                buffer = result["error"].cast<std::string>();
                return buffer.c_str();
            }
            return buffer.c_str();
        }

        callback_t install_deps(status_callback_t status_cb)
        {
            return [status_cb]
            {
                try {
                    dexpert::py::getModule().attr("install_dependencies")();
                    status_cb(true, NULL); // TODO: check error!
                }
                catch(std::runtime_error e) {
                    status_cb(false, getError(e)); // TODO: check error!
                }
            };
        }

        callback_t check_have_deps(status_callback_t status_cb)
        {
            return [status_cb]
            {
                try {
                    auto r = dexpert::py::getModule().attr("have_dependencies")();
                    status_cb(r.cast<py11::bool_>(), NULL); // TODO: check error!
                }
                catch(std::runtime_error e) {
                    status_cb(false, getError(e)); // TODO: check error!
                }
            };
        }

        callback_t open_image(const char *path, image_callback_t status_cb)
        {
            return [status_cb, path]
            {
                try {
                    auto r = dexpert::py::getModule().attr("open_image")(path);
                    py11::dict d = r.cast<py11::dict>();
                    auto img = dexpert::py::rawImageFromPyDict(d);
                    status_cb(true, NULL, img); // TODO: check error!
                } catch(std::runtime_error e) {
                    status_cb(false, getError(e), image_ptr_t()); // TODO: check error!
                }
            };
        }

        callback_t save_image(const char *path, RawImage *image, status_callback_t status_cb)
        {
            return [status_cb, path, image]()
            {
                try {
                    py11::dict d;
                    image->toPyDict(d);
                    dexpert::py::getModule().attr("save_image")(path, d);
                    status_cb(true, NULL); // TODO: check error!
                }
                catch(std::runtime_error e) {
                    status_cb(false, getError(e)); // TODO: check error!
                }
            };
        }

        callback_t pre_process_image(const char *mode, RawImage *image, image_callback_t status_cb)
        {
            enable_progress_window(false);
            return [status_cb, mode, image]()
            {
                try {
                    py11::dict d;
                    image->toPyDict(d);
                    auto r = dexpert::py::getModule().attr("pre_process_image")(mode, d);
                    py11::dict asimg = r.cast<py11::dict>();
                    auto img = dexpert::py::rawImageFromPyDict(asimg);
                    status_cb(true, NULL, img); // TODO: check error!
                }
                catch(std::runtime_error e) {
                    status_cb(false, getError(e), image_ptr_t()); // TODO: check error!
                }
            };
        }

        callback_t upscale_image(RawImage *image, float scale, image_callback_t status_cb)
        {
            enable_progress_window(false);
            return [status_cb, image, scale]
            {
                try {
                    py11::dict d;
                    py11::dict params;
                    image->toPyDict(d);
                    auto r = dexpert::py::getModule().attr("gfpgan_upscale")(d, py11::float_(scale), params);
                    py11::dict d2 = r.cast<py11::dict>();
                    auto img = dexpert::py::rawImageFromPyDict(d2);
                    status_cb(true, NULL, img); // TODO: check error!
                } catch(std::runtime_error e) {
                    status_cb(false, getError(e), image_ptr_t()); // TODO: check error!
                }
            };
        }

        const void txt2img_config_t::fill_prompt_dict(py11::dict &params) const
        {
            params["prompt"] = this->prompt;
            params["negative"] = this->negative;
            params["model"] = this->model;
            params["width"] = this->width;
            params["height"] = this->height;
            params["steps"] = this->steps;
            params["cfg"] = this->cfg;
            params["seed"] = this->seed;
            params["variation"] = this->variation;
            params["var_stren"] = this->var_stren;
            params["restore_faces"] = restore_faces;
            params["enable_codeformer"] = enable_codeformer;

            if (!this->controlnets.empty())
            {
                py11::list arr(0);

                for (auto it = this->controlnets.begin(); it != this->controlnets.end(); it++)
                {
                    py11::dict c;
                    py11::dict data;
                    it->image->toPyDict(data);
                    c["mode"] = it->mode;
                    c["image"] = data;
                    c["strength"] = it->strength;
                    arr.append(c);
                }
                params["controlnets"] = arr;
            }
        }

        const void img2img_config_t::fill_prompt_dict(py11::dict &params) const
        {
            txt2img_config_t::fill_prompt_dict(params);
            py11::dict data;
            this->image->toPyDict(data);
            params["image"] = data;
            params["strength"] = this->strength;

            if (this->mask)
            {
                py11::dict data;
                this->mask->toPyDict(data);
                params["mask"] = data;
            }
        }

        callback_t get_diffusion_callback(const char *fn_name, const txt2img_config_t &config, image_callback_t status_cb)
        {
            return [fn_name, &config, status_cb]
            {
                try {
                    py11::dict params;
                    config.fill_prompt_dict(params);
                    auto r = dexpert::py::getModule().attr(fn_name)(params);
                    py11::dict asimg = r.cast<py11::dict>();
                    auto img = dexpert::py::rawImageFromPyDict(asimg);
                    status_cb(true, errorFromPyDict(asimg, "Error generating the image"), img); // TODO: check error!
                } catch(std::runtime_error e) {
                    static std::string es;
                    es = e.what();
                    status_cb(false, es.c_str(), image_ptr_t()); // TODO: check error!
                }
            };
        }

        callback_t txt2_image(const txt2img_config_t &config, image_callback_t status_cb)
        {
            enable_progress_window();
            return get_diffusion_callback("txt2img", config, status_cb);
        }

        callback_t img2_image(const img2img_config_t &config, image_callback_t status_cb)
        {
            enable_progress_window();
            return get_diffusion_callback("img2img", config, status_cb);
        }

        callback_t list_models(const std::wstring &path, model_callback_t status_cb)
        {
            return [&path, status_cb]
            {
                try {
                    model_list_t models;

                    auto r = dexpert::py::getModule().attr("list_models")(path);
                    auto seq = r.cast<py11::sequence>();
                    for (size_t i = 0; i < seq.size(); ++i)
                    {
                        auto it = seq[i].cast<py11::dict>();
                        model_t model;
                        model.name = it["name"].cast<std::string>();
                        model.size = it["size"].cast<py11::int_>();
                        model.hash = it["hash"].cast<std::string>();
                        model.path = it["path"].cast<std::string>();
                        models.push_back(model);
                    }
                    status_cb(true, NULL, models); // TODO: check error!
                } catch(std::runtime_error e) {
                    status_cb(false, getError(e), model_list_t()); // TODO: check error!
                }

            };
        }

        callback_t model_urls(model_url_callback_t status_cb) {
            return [status_cb]
            {
                try {
                    model_url_list_t models;
                    auto r = dexpert::py::getModule().attr("get_sd_model_urls")();
                    auto seq = r.cast<py11::sequence>();
                    for (size_t i = 0; i < seq.size(); ++i)
                    {
                        auto it = seq[i].cast<py11::dict>();
                        model_urls_t model;
                        model.url = it["url"].cast<std::string>();
                        model.displayName = it["display_name"].cast<std::string>();
                        model.description = it["description"].cast<std::string>();
                        model.filename = it["filename"].cast<std::string>();
                        models.push_back(model);
                    }
                    status_cb(true, NULL, models); // TODO: check error!
                } catch(std::runtime_error e) {
                    status_cb(false, getError(e), model_url_list_t()); // TODO: check error!
                }
            };
        }

        callback_t download_model(const char *url, const char *filename, status_callback_t status_cb)
        {
            enable_progress_window(false);
            return [status_cb, url, filename]()
            {
                try {
                    dexpert::py::getModule().attr("download_sd_model")(url, filename);
                    status_cb(true, NULL); // TODO: check error!
                }
                catch(std::runtime_error e) {
                    status_cb(false, getError(e)); // TODO: check error!
                }
            };
        }

        callback_t configure_stable_diffusion(status_callback_t status_cb)
        {
            return [status_cb]
            {
                try {
                    py11::dict settings;
                    auto &c = getConfig();
                    settings["scheduler"] = c.getScheduler();
                    settings["nsfw_filter"] = c.getSafeFilter() ? 1 : 0;
                    settings["device"] = c.getUseGPU() ? "cuda" : "cpu";
                    settings["use_float16"] = c.getUseFloat16();
                    settings["gfpgan.arch"] = c.gfpgan_get_arch();
                    settings["gfpgan.channel_multiplier"] = c.gfpgan_get_channel_multiplier();
                    settings["gfpgan.has_aligned"] = c.gfpgan_get_has_aligned();
                    settings["gfpgan.only_center_face"] = c.gfpgan_get_only_center_face();
                    settings["gfpgan.paste_back"] = c.gfpgan_get_paste_back();
                    settings["gfpgan.weight"] = c.gfpgan_get_weight();
                    auto r = dexpert::py::getModule().attr("set_user_settings")(settings);
                    status_cb(true, NULL);
                } catch(std::runtime_error e) {
                    status_cb(false, getError(e)); // TODO: check error!
                }
            };
        }

    } // namespace py
} // namespace
