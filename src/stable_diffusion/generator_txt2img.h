/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_
#define SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_

#include <string>
#include "src/stable_diffusion/generator.h"

namespace dexpert
{

class GeneratorTxt2Image: public GeneratorBase {
    public:
        GeneratorTxt2Image(
            const std::string& prompt,
            const std::string& negative,
            const std::wstring& model,
            int seed = -1,
            size_t width = 512,
            size_t height = 512,
            size_t steps = 50,
            float cfg = 7.5
        );

        void generate(
            generator_cb_t cb,
            int seed_index,
            int variation, 
            float var_factor,
            bool enable_variation = false
        ) override;

    private:
        std::string prompt_;
        std::string negative_;
        std::wstring model_;
        int seed_ = -1;
        size_t width_ = 512;
        size_t height_ = 512;
        size_t steps_ = 50;
        float cfg_ = 7.5;
        image_ptr_t image_;
};


}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_