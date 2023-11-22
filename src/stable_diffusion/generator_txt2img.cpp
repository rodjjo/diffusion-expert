#include "src/stable_diffusion/generator_txt2img.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"


namespace dexpert
{

 GeneratorTxt2Image::GeneratorTxt2Image(
        std::shared_ptr<SeedGenerator> seed_gen, 
        bool variation,
        const std::string& prompt,
        const std::string& negative,
        const std::string& model,
        controlnet_list_t controlnets,
        int batch_size,
        int seed,
        size_t width,
        size_t height,
        size_t steps,
        float cfg,
        float var_stren,
        bool restore_faces,
        bool enable_codeformer,
        bool reload_model,
        bool use_lcm
    ) : GeneratorBase(seed_gen, variation), prompt_(prompt), negative_(negative), model_(model), controlnets_(controlnets),
        seed_(seed), batch_size_(batch_size), width_(width), height_(height), steps_(steps), 
        cfg_(cfg), var_strength_(var_stren), 
        restore_faces_(restore_faces), enable_codeformer_(enable_codeformer), reload_model_(reload_model), use_lcm_(use_lcm)
        {
}

std::shared_ptr<GeneratorBase> GeneratorTxt2Image::duplicate(bool variation, image_ptr_t img) {
    std::shared_ptr<GeneratorTxt2Image> d;
    int seed = variation ? this->seed_ : this->getSeedGenerator()->newSeed();
    d.reset(new GeneratorTxt2Image(
        this->getSeedGenerator(),
        variation,
        this->prompt_,
        this->negative_,
        this->model_,
        this->controlnets_,
        this->batch_size_,
        seed,
        this->width_,
        this->height_,
        this->steps_,
        this->cfg_,
        this->var_strength_,
        this->restore_faces_,
        this->enable_codeformer_,
        false, // only the first one should reload the model
        this->use_lcm_
    ));
    if (img) {
        d->setImage(img);
    }
    return d;
}

void GeneratorTxt2Image::generate(generator_cb_t cb) {
    bool success = false;

    const char *message = "Unexpected error. Callback to generate image not called";
    std::list<image_ptr_t> images;

    dexpert::py::txt2img_config_t params;

    params.prompt = prompt_.c_str();
    params.negative = negative_.c_str();
    params.model = model_.c_str();
    params.batch_size = batch_size_;
    params.seed = seed_;
    params.variation = isVariation() ? rand() : 0;
    params.var_stren = var_strength_;
    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;
    params.restore_faces = restore_faces_;
    params.enable_codeformer = enable_codeformer_;
    params.reload_model = reload_model_;
    params.use_lcm = use_lcm_;
    reload_model_ = false;

    for (auto it = controlnets_.begin(); it != controlnets_.end(); it++) {
        dexpert::py::control_net_t control;
        control.strength = (*it)->getStrenght();
        control.mode = (*it)->getMode();
        control.image = (*it)->getImage();
        params.controlnets.push_back(
            control
        );
    } 

    if (!isVariation()) {
        params.var_stren = 0;
    }

    auto gen_cb = dexpert::py::txt2_image(params, [&images, &success, &message] (bool status, const char* msg, std::list<image_ptr_t> imgs) {
        success = status;
        message = msg;
        images = imgs;
    });

    dexpert::py::get_py()->execute_callback(gen_cb);

    if (images.size()) {
        auto img = *images.begin();
        if (img->w() != width_ || img->h() != height_) {
            setImage(img->getCrop(0, 0, width_, height_));
        } else {
            setImage(img);
        }
    }

    for (auto & img: images) {
        if (img->w() != width_ || img->h() != height_) {
            auto i = img->getCrop(0, 0, width_, height_);
            img.swap(i);
        }
    }

    cb(success, message, images);
}

int GeneratorTxt2Image::batchSize() {
    return batch_size_;
}

} // namespace dexpert

