/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_GENERATOR_IMG2IMG_H
#define SRC_STABLE_DIFFUSION_GENERATOR_IMG2IMG_H

#include <memory>
#include <string>
#include <list>
#include "src/python/raw_image.h"
#include "src/stable_diffusion/generator.h"
#include "src/stable_diffusion/controlnet.h"

namespace dexpert
{

typedef std::list<std::shared_ptr<ControlNet> > controlnet_list_t;

class GeneratorImg2Image: public GeneratorBase {
    public:
        GeneratorImg2Image(
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
        );

        void generate(
            generator_cb_t cb
        ) override;

        std::shared_ptr<GeneratorBase> duplicate(bool variation);
        
    private:
        image_ptr_t image_;
        image_ptr_t mask_;
        std::string prompt_;
        std::string negative_;
        std::string model_;
        controlnet_list_t controlnets_;
        int seed_ = -1;
        size_t image_orig_w_ = 512;
        size_t image_orig_h_ = 512;
        size_t width_ = 512;
        size_t height_ = 512;
        size_t steps_ = 50;
        float cfg_ = 7.5;
        float var_strength_ = 0.1;
        float image_strength_ = 0.8;
        float mask_blur_size_ = 4.0;
        inpaint_mode_t inpaint_mode_;
        bool restore_faces_ = false;
        bool reload_model_ = false;
        bool enable_codeformer_ = false;
};


}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_IMG2IMG_H