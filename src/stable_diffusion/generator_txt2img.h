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
            float cfg = 7.5,
            float var_stren = 0.1
        );

        void generate(
            generator_cb_t cb,
            int seed_index,
            int enable_variation = 0
        ) override;

        
        std::shared_ptr<GeneratorBase> duplicate();
        
    private:
        std::string prompt_;
        std::string negative_;
        std::wstring model_;
        int seed_ = -1;
        size_t width_ = 512;
        size_t height_ = 512;
        size_t steps_ = 50;
        float cfg_ = 7.5;
        float var_strenght_ = 0.1;
};


}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_