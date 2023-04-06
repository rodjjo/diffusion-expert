/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_STATE_H_
#define SRC_STABLE_DIFFUSION_STATE_H_

#include <string>
#include <list>
#include <memory>

#include "src/python/raw_image.h"

namespace dexpert
{

class StableDiffusionState;
std::shared_ptr<StableDiffusionState> get_sd_state();

typedef struct {
    std::string name;
    std::string hash;
    std::wstring path;
    size_t model_size;
} model_info_t;

class StableDiffusionState {
 private:
    friend std::shared_ptr<StableDiffusionState> get_sd_state();
    StableDiffusionState();

 public:
    virtual ~StableDiffusionState();
    StableDiffusionState (const StableDiffusionState &) = delete;
    StableDiffusionState & operator = (const StableDiffusionState &) = delete;

    // models
    bool reloadSdModelList();
    const std::list<model_info_t> &getSdModels() const;
    void setSdModel(const std::string& name);

    // prompts
    void setPrompt(const char *prompt);
    void setNegativePrompt(const char *prompt);
    const char *getPrompt();
    const char *getNegativePrompt();
    int getSeed();
    int getWidth();
    int getHeight();
    int getSteps();
    float getCFG();
    void setSeed(int value);
    void setWidth(int value);
    void setHeight(int value);
    void setSteps(int value);
    void setCFG(float value);

    // input images
    bool generateInputImage();
    bool openInputImage(const char *path);
    void setInputImage(image_ptr_t image);
    bool saveInputImage(const char *path);
    RawImage *getInputImage();
    const char* lastError();

private:
    int seed_ = -1;
    int width_ = 512;
    int height_ = 512;
    int steps_ = 50;
    float cfg_ = 7.5;
    std::list<model_info_t> sdModels_;
    std::wstring currentSdModel_;
    std::string prompt_;
    std::string negative_prompt_;
    std::string last_error_;
    image_ptr_t input_image_;
};
    
} // namespace dexpert


#endif // SRC_STABLE_DIFFUSION_STATE_H_