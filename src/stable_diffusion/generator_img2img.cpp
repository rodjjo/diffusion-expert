#include "src/stable_diffusion/generator_img2img.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"


namespace dexpert
{

namespace 
{
    const char *inpaint_mode_names[inpaint_mode_count] = {
        "original",
        "fill",
        "original", // wholepicture + original
        "fill" // wholepicture + fill
        // "latent",
        // "nothing"
    };
} // unamed namespace 


 GeneratorImg2Image::GeneratorImg2Image(
        std::shared_ptr<SeedGenerator> seed_gen, 
        bool variation,
        const std::string& prompt,
        const std::string& negative,
        const std::string& model,
        controlnet_list_t controlnets,
        image_ptr_t image,
        image_ptr_t mask,
        int seed,
        size_t width,
        size_t height,
        size_t steps,
        float cfg,
        float var_stren,
        float image_strength,
        bool restore_faces,
        bool enable_codeformer,
        bool reload_model,
        float mask_blur_size,
        inpaint_mode_t inpaint_mode
    ) : 
        GeneratorBase(seed_gen, variation),
        prompt_(prompt), 
        negative_(negative), 
        model_(model), 
        controlnets_(controlnets),
        image_(image),
        mask_(mask),
        seed_(seed), 
        width_(width), 
        height_(height), 
        steps_(steps), 
        cfg_(cfg), 
        var_strength_(var_stren),
        image_strength_(image_strength),
        restore_faces_(restore_faces), 
        enable_codeformer_(enable_codeformer),
        reload_model_(reload_model),
        mask_blur_size_(mask_blur_size),
        inpaint_mode_(inpaint_mode)
        {
    image_orig_w_ = image_->w();
    image_orig_h_ = image_->h();

    if (image_ && (image_->w() % 8 > 0 || image_->h() % 8 > 0)) {
        image_ = image_->ensureMultipleOf8();
    }
    if (mask_ && (mask_->w() % 8 > 0 || mask_->h() % 8 > 0)) {
        mask_ = mask_->ensureMultipleOf8();
    }
}

std::shared_ptr<GeneratorBase> GeneratorImg2Image::duplicate(bool variation) {
    std::shared_ptr<GeneratorImg2Image> d;
    int seed = variation ? this->seed_ : this->getSeedGenerator()->newSeed();
    d.reset(new GeneratorImg2Image(
        this->getSeedGenerator(),
        variation,
        this->prompt_,
        this->negative_,
        this->model_,
        this->controlnets_,
        this->image_,
        this->mask_,
        seed,
        this->width_,
        this->height_,
        this->steps_,
        this->cfg_,
        this->var_strength_,
        this->image_strength_,
        this->restore_faces_,
        this->enable_codeformer_,
        false, // only the first one should reload the model
        this->mask_blur_size_,
        this->inpaint_mode_
    ));
    return d;
}

void GeneratorImg2Image::generate(generator_cb_t cb) {
    bool success = false;

    const char *message = "Unexpected error. Callback to generate image not called";
    dexpert::py::img2img_config_t params;

    image_ptr_t blur_mask;
    image_ptr_t new_image;

    params.prompt = prompt_.c_str();
    params.negative = negative_.c_str();
    params.model = model_.c_str();
    params.seed = seed_;
    params.variation = isVariation() ? rand() : 0;
    params.var_stren = var_strength_;
    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;
    params.image = image_.get();
    params.inpaint_mode = inpaint_mode_names[inpaint_mode_];
    params.reload_model = reload_model_;
    reload_model_ = false;

    if (mask_) {
        if (mask_blur_size_) {
            blur_mask = mask_->blur(mask_blur_size_);
        }
    }

    image_ptr_t full_mask = blur_mask;
    if (inpaint_mode_ == inpaint_wholepicture) {
        full_mask = dexpert::py::newImage(full_mask->w(), full_mask->h(), false);
        full_mask = full_mask->removeAlpha();
    }
    
    params.mask = full_mask.get();
    params.strength = image_strength_;
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

    if (!isVariation()) {
        params.var_stren = 0;
    }
    
    image_ptr_t result;
    auto gen_cb = dexpert::py::img2_image(params, [&result, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
        success = status;
        message = msg;
        result = img;
    });

    dexpert::py::get_py()->execute_callback(gen_cb);

    if (result) {
        if (mask_.get() != NULL && image_.get() != NULL) {
            if (mask_blur_size_) {
                blur_mask = mask_->removeAlpha()->blur(mask_blur_size_)->resizeCanvas(image_->w(), image_->h());
            } 
            // I do not want stable diffusion to change non masked pixels
            // auto invert = mask_->resizeCanvas(image_->w(), image_->h())->removeAlpha();
            result->pasteAt(0, 0, blur_mask.get(), image_.get());
        }

        if (result->w() != image_orig_w_ || image_->h() != image_orig_h_) {
            result = result->getCrop(0, 0, image_orig_w_, image_orig_h_);
        }

        setImage(result);
    }

    cb(success, message, result);
}


} // namespace dexpert

