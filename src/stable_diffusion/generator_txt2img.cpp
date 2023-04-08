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
            float cfg
        ) : prompt_(prompt), negative_(negative), model_(model),
            seed_(seed), width_(width), height_(height), cfg_(cfg)
         {

}

void GeneratorTxt2Image::generate(generator_cb_t cb, int seed_index, int variation,  float var_factor) {
    bool success = false;

    const char *message = "Unexpected error. Callback to generate image not called";
    image_ptr_t image;
    dexpert::py::txt2img_config_t params;
    params.prompt = prompt_.c_str();
    params.negative = negative_.c_str();
    params.model = model_.c_str();
    params.seed = seed_ + seed_index;
    params.variation = 0;
    params.var_step = var_factor;

    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;
    
    auto gen_cb = dexpert::py::txt2_image(params, [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
        success = status;
        message = msg;
        image = img;
    });

    dexpert::py::get_py()->execute_callback(gen_cb);
    
    if (!image_) {
        image_ = image; // keep the image for variation purpose
    }

    cb(success, message, image);
}


} // namespace dexpert

