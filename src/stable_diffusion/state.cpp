#include "src/stable_diffusion/state.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"

namespace dexpert {
namespace {

const char *kNO_ERROR_MESSAGE = "Error with no error message";

std::shared_ptr<StableDiffusionState> sd_state;

}  // unnamed namespace

std::shared_ptr<StableDiffusionState> get_sd_state() {
    if (!sd_state) {
        sd_state.reset(new StableDiffusionState());
    }
    return sd_state;
}

StableDiffusionState::StableDiffusionState() {

}

StableDiffusionState::~StableDiffusionState() {

}

bool StableDiffusionState::reloadSdModelList() {
    bool success = false;
    last_error_ = std::string();
    const char *msg = "Fail retriving model list";
    std::list<model_info_t> new_list;
    auto cb = [&success, &msg, &new_list] (bool s, const char *m, const dexpert::py::model_list_t &l) {
        if (s) {
            success = true;
            for (auto it = l.cbegin(); it != l.cend(); it++) {
                model_info_t el;
                el.hash = it->hash;
                el.model_size = it->size;
                el.name = it->name;
                el.path = it->path;
                new_list.push_back(el);
            }
        } else {
            if (m) msg = m;
        }
    };

    dexpert::py::get_py()->execute_callback(dexpert::py::list_models(
        getConfig().sdModelsDir().c_str(),
        cb
    ));

    sdModels_ = new_list;

    if (!success) {
        last_error_ = msg;
    } else if (currentSdModel_.empty() && !sdModels_.empty()) {
        setSdModel(sdModels_.rbegin()->name);
    }

    return success;
}

const std::list<model_info_t> &StableDiffusionState::getSdModels() const {
    return sdModels_;
}

void StableDiffusionState::setSdModel(const std::string& name) {
    for (auto it = sdModels_.cbegin(); it != sdModels_.cend(); it++) {
        if (name == it->name) {
            currentSdModel_ = it->path;
            getConfig().setLastSdModel(name);
            getConfig().save();
            break;
        }
    }
}

bool StableDiffusionState::generateInputImage() {
    bool success = false;
    last_error_  = std::string();
    std::string prompt = prompt_;
    std::string negative = negative_prompt_;

    const char *message = "Unexpected error. Callback to generate image not called";
    image_ptr_t image;
    dexpert::py::txt2img_config_t params;
    params.prompt = prompt.c_str();
    params.negative = negative.c_str();
    params.model = currentSdModel_.c_str();
    params.seed = seed_;
    params.steps = steps_;
    params.cfg = cfg_;
    params.width = width_;
    params.height = height_;
    
    auto cb = dexpert::py::txt2_image(params, [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
        success = status;
        message = msg;
        image = img;
    });

    dexpert::py::get_py()->execute_callback(cb);

    if (!success) {
        last_error_ = message ? message : kNO_ERROR_MESSAGE;
    } else if (image) {
        input_image_ = image;
        return true;
    }

    return false;
}

bool StableDiffusionState::openInputImage(const char *path) {
    bool success = false;
    last_error_ = std::string();

    const char *message = kNO_ERROR_MESSAGE;
    image_ptr_t image;
    auto cb = dexpert::py::open_image(path, [&image, &success, &message] (bool status, const char* msg, image_ptr_t img) {
        success = status;
        message = msg;
        image = img;
    });

    dexpert::py::get_py()->execute_callback(cb);

    if (!success) {
        last_error_ = message ? message : kNO_ERROR_MESSAGE;
    } else if (image) {
        input_image_ = image;
    }

    return success;
}

bool StableDiffusionState::saveInputImage(const char *path) {
    if (!input_image_) {
        last_error_ = "no image to save";
        return false;
    }

    last_error_  = std::string();
    bool success = false;
    const char *message = kNO_ERROR_MESSAGE;

    auto cb = dexpert::py::save_image(path, input_image_, [&success, &message] (bool status, const char* msg) {
        success = status;
        message = msg;
    });

    dexpert::py::get_py()->execute_callback(cb);

    if (!success) {
        last_error_ = message ? message : kNO_ERROR_MESSAGE;
        return false;
    }

    return success;
}

void StableDiffusionState::setInputImage(image_ptr_t image) {
    input_image_ = image;
}
    
RawImage *StableDiffusionState::getInputImage() {
    return input_image_.get();
}

const char* StableDiffusionState::lastError() {
    return last_error_.c_str();
}

void StableDiffusionState::setPrompt(const char *prompt) {
    prompt_ = prompt;
}

void StableDiffusionState::setNegativePrompt(const char *prompt) {
    negative_prompt_ = prompt;
}

const char *StableDiffusionState::getPrompt() {
    return prompt_.c_str();
}

const char *StableDiffusionState::getNegativePrompt() {
    return negative_prompt_.c_str();
}

int StableDiffusionState::getSeed() {
    return seed_;
}

void StableDiffusionState::setWidth(int value) {
    width_ = value;
}

void StableDiffusionState::setHeight(int value) {
    height_ = value;
}

void StableDiffusionState::setSteps(int value) {
    steps_ = value;
}
void StableDiffusionState::setCFG(float value) {
    cfg_ = value;
}

void StableDiffusionState::setSeed(int value) {
    seed_ = value;
}

int StableDiffusionState::getWidth() {
    return width_;
}

int StableDiffusionState::getHeight() {
    return height_;
}

int StableDiffusionState::getSteps() {
    return steps_;
}

float StableDiffusionState::getCFG() {
    return cfg_;
}
    
} // namespace dexpert
