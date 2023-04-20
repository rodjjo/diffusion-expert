/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_HELPERS_H_
#define SRC_PYTHON_HELPERS_H_

#include <list>
#include <string>
#include <memory>
#include <functional>

#include <Python.h>

#include "src/python/raw_image.h"
#include "src/python/guard.h"

namespace dexpert {
namespace py {

typedef struct  {
    std::string name;
    std::string hash;
    std::wstring path;
    size_t size;
} model_t;

typedef std::list<model_t> model_list_t;

typedef struct {
    const char *mode = "";
    RawImage *image = NULL;
    float strength = 1.0;
} control_net_t;

class txt2img_config_t {
 public:
    const char *prompt = "";
    const char *negative = "";
    const wchar_t *model = L"";
    int seed = -1;
    size_t width = 512;
    size_t height = 512;
    size_t steps = 50;
    float cfg = 7.5;
    int variation = 0;
    float var_stren = 0.01;
    bool restore_faces = false;
    bool enable_codeformer = false;
    std::list<control_net_t> controlnets;
    virtual ~txt2img_config_t() {};
    virtual const void fill_prompt_dict(PyObject *params, ObjGuard &guard) const;
};

class img2img_config_t: public txt2img_config_t {
 public:
    RawImage *image = NULL;
    RawImage *mask = NULL;
    float strength = 0.8;
    bool invert_mask = true;
    const void fill_prompt_dict(PyObject *params, ObjGuard &guard) const override;
};



typedef std::function<void()> callback_t;
typedef std::function<void(bool success, const char *message)> status_callback_t;
typedef std::function<void(bool success, const char *message, std::shared_ptr<RawImage> image)> image_callback_t;
typedef std::function<void(bool success, const char *message, const model_list_t &models)> model_callback_t;

callback_t check_have_deps(status_callback_t status_cb);
callback_t install_deps(status_callback_t status_cb);
callback_t configure_stable_diffusion(status_callback_t status_cb);

callback_t open_image(const char* path, image_callback_t status_cb);
callback_t save_image(const char* path, RawImage *image, status_callback_t status_cb);
callback_t pre_process_image(const char *mode, RawImage *image, image_callback_t status_cb);
callback_t txt2_image(const txt2img_config_t& config, image_callback_t status_cb); 
callback_t img2_image(const img2img_config_t& config, image_callback_t status_cb); 

callback_t list_models(const wchar_t* path, model_callback_t status_cb);

}  // namespace py
}  // namespace

#endif  // SRC_PYTHON_HELPERS_H_
