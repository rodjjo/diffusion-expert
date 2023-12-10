#pragma once

#include <pybind11/embed.h> 
#include <vector>
#include <string>
#include <functional>

#include "python/code.h"
#include "python/image.h"

namespace py11 = pybind11;

namespace dfe
{

class ControlnetParameters {
public:
    std::string mode;
    image_ptr_t image;
    py11::dict toDict();
};

class LoraParameters {
public:
    std::string model;
    float weight = 1.0;

    py11::dict toDict();
};

class GeneratorParameters {
 public:
    std::string positive_prompt;
    std::string negative_prompt;
    std::string mode;
    std::string model;
    std::string inpaint_model;
    std::string scheduler_name;
    float cfg = 7.1;
    int seed = -1;
    int width = 512;
    int height = 512;
    int steps = 25;
    int batch_size = 1;
    bool use_lcm_lora = false;
    bool use_tiny_vae = false;
    image_ptr_t image;
    image_ptr_t mask;
    std::vector<ControlnetParameters> controlnets;
    std::vector<LoraParameters> lora_list;
    py11::dict toDict();
};
    
} // namespace dfe
