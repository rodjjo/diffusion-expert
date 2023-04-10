#include "src/stable_diffusion/generator_txt2img.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"


namespace dexpert
{

 GeneratorTxt2Image::GeneratorTxt2Image(
            const std::string& prompt,
            const std::string& negative,
            const std::wstring& model,
            int seed,
            size_t width,
            size_t height,
            size_t steps,
            float cfg,
            float var_stren
        ) : prompt_(prompt), negative_(negative), model_(model), 
            seed_(seed), width_(width), height_(height), steps_(steps), 
            cfg_(cfg), var_strenght_(var_stren)
         {

}

std::shared_ptr<GeneratorBase> GeneratorTxt2Image::duplicate() {
    std::shared_ptr<GeneratorTxt2Image> d;
    d.reset(new GeneratorTxt2Image(
            this->prompt_,
            this->negative_,
            this->model_,
            this->seed_,
            this->width_,
            this->height_,
            this->steps_,
            this->cfg_,
            this->var_strenght_
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
    params.var_stren = var_strenght_;
    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;

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

