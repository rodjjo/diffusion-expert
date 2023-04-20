#include "src/stable_diffusion/generator_txt2img.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"


namespace dexpert
{

 GeneratorTxt2Image::GeneratorTxt2Image(
        const std::string& prompt,
        const std::string& negative,
        const std::wstring& model,
        controlnet_list_t controlnets,
        int seed,
        size_t width,
        size_t height,
        size_t steps,
        float cfg,
        float var_stren,
        bool restore_faces,
        bool enable_codeformer
    ) : prompt_(prompt), negative_(negative), model_(model), controlnets_(controlnets),
        seed_(seed), width_(width), height_(height), steps_(steps), 
        cfg_(cfg), var_strength_(var_stren), 
        restore_faces_(restore_faces), enable_codeformer_(enable_codeformer)
        {

}

std::shared_ptr<GeneratorBase> GeneratorTxt2Image::duplicate() {
    std::shared_ptr<GeneratorTxt2Image> d;
    d.reset(new GeneratorTxt2Image(
        this->prompt_,
        this->negative_,
        this->model_,
        this->controlnets_,
        this->seed_,
        this->width_,
        this->height_,
        this->steps_,
        this->cfg_,
        this->var_strength_,
        this->restore_faces_,
        this->enable_codeformer_
    ));
    return d;
}

void GeneratorTxt2Image::generate(generator_cb_t cb, int seed_index, int enable_variation) {
    bool success = false;

    const char *message = "Unexpected error. Callback to generate image not called";
    image_ptr_t image;

    dexpert::py::txt2img_config_t params;

    params.prompt = prompt_.c_str();
    params.negative = negative_.c_str();
    params.model = model_.c_str();
    params.seed = seed_ + seed_index;
    params.variation = enable_variation == 0 ? 0 : computeVariationSeed(enable_variation < 0);
    params.var_stren = var_strength_;
    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;
    params.restore_faces = restore_faces_;
    params.enable_codeformer = enable_codeformer_;

    for (auto it = controlnets_.begin(); it != controlnets_.end(); it++) {
        dexpert::py::control_net_t control;
        control.strength = (*it)->getStrenght();
        control.mode = (*it)->getMode();
        control.image = (*it)->getImage();
        params.controlnets.push_back(
            control
        );
    } 

    if (enable_variation == 0) {
        params.var_stren = 0;
    }

    auto gen_cb = dexpert::py::txt2_image(params, [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
        success = status;
        message = msg;
        image = img;
    });

    dexpert::py::get_py()->execute_callback(gen_cb);

    if (image) {
        if (enable_variation == 0) {
            setImage(image, params.seed);
        } else  {
            addVariation(image, params.variation, enable_variation < 0);
        }
    }

    cb(success, message, image);
}


} // namespace dexpert

