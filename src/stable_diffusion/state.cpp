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

bool StableDiffusionState::generateInputImage() {
    bool success = false;
    last_error_  = std::string();

    std::wstring model = dexpert::getConfig().sdModelsDir() + L"/model.safetensors";
    const char *message = "Unexpected error. Callback to generate image not called";
    image_ptr_t image;
    dexpert::py::txt2img_config_t cfg;
    cfg.prompt = L"An astrounaut riding a horse at the moon";
    cfg.negative = L"drawing,cartoon,3d,render,rendering";
    cfg.model = model.c_str();
    
    auto cb = dexpert::py::txt2_image(cfg, [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
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

    
} // namespace dexpert
