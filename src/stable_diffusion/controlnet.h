/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_CONTROLNET_H_
#define SRC_STABLE_DIFFUSION_CONTROLNET_H_

#include <stdint.h>
#include <string>
#include "src/python/raw_image.h"

namespace dexpert
{

class ControlNet {
 public:
    ControlNet(const char *mode, image_ptr_t image, float strength=1.0);
    virtual ~ControlNet();

    float getStrenght();
    const char *getMode();
    RawImage *getImage();
    void resizeImage(int w, int h);
 private:
    std::string mode_;
    image_ptr_t image_;
    float strength_;
};

} // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_CONTROLNET_H_
