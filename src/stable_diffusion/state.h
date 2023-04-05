/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_STATE_H_
#define SRC_STABLE_DIFFUSION_STATE_H_

#include <string>
#include <memory>

#include "src/python/raw_image.h"

namespace dexpert
{

class StableDiffusionState;
std::shared_ptr<StableDiffusionState> get_sd_state();

class StableDiffusionState {
 private:
    friend std::shared_ptr<StableDiffusionState> get_sd_state();
    StableDiffusionState();

 public:
    virtual ~StableDiffusionState();
    StableDiffusionState (const StableDiffusionState &) = delete;
    StableDiffusionState & operator = (const StableDiffusionState &) = delete;

    // prompts
    void setPrompt(const char *prompt);
    void setNegativePrompt(const char *prompt);
    const char *getPrompt();
    const char *getNegativePrompt();

    // input images
    bool generateInputImage();
    bool openInputImage(const char *path);
    void setInputImage(image_ptr_t image);
    bool saveInputImage(const char *path);
    RawImage *getInputImage();
    const char* lastError();

private:
    std::string prompt_;
    std::string negative_prompt_;
    std::string last_error_;
    image_ptr_t input_image_;
};
    
} // namespace dexpert


#endif // SRC_STABLE_DIFFUSION_STATE_H_