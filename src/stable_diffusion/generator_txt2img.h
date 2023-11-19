/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_
#define SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_

#include <memory>
#include <string>
#include <list>
#include "src/stable_diffusion/generator.h"
#include "src/stable_diffusion/controlnet.h"

namespace dexpert
{

typedef std::list<std::shared_ptr<ControlNet> > controlnet_list_t;

class GeneratorTxt2Image: public GeneratorBase {
    public:
        GeneratorTxt2Image(
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
            bool reload_model
        );

        void generate(
            generator_cb_t cb
        ) override;

        int batchSize() override;

        
        std::shared_ptr<GeneratorBase> duplicate(bool variation, image_ptr_t img) override;
        
    private:
        std::string prompt_;
        std::string negative_;
        std::string model_;
        controlnet_list_t controlnets_;
        int seed_ = -1;
        int batch_size_ = 1;
        size_t width_ = 512;
        size_t height_ = 512;
        size_t steps_ = 50;
        float cfg_ = 7.5;
        float var_strength_ = 0.1;
        bool restore_faces_ = false;
        bool enable_codeformer_ = false;
        bool reload_model_ = false;
};


}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_