/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_GENERATOR_H_
#define SRC_STABLE_DIFFUSION_GENERATOR_H_

#include <functional>
#include "src/python/raw_image.h"

namespace dexpert
{

typedef std::function<void(bool success, const char* msg, image_ptr_t result)> generator_cb_t;

class GeneratorBase {
  public:
    virtual ~GeneratorBase(){}
    virtual void generate(
            generator_cb_t cb,
            int seed_index,
            int variation, 
            float var_factor,
            bool enable_variation = false
    ) = 0;
};
    
}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_