#include "src/stable_diffusion/state.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"

namespace dexpert {
namespace {

const char *kNO_ERROR_MESSAGE = "Error with no error message";
const int MAX_GENERATORS = 4;

std::shared_ptr<StableDiffusionState> sd_state;

}  // unnamed namespace

std::shared_ptr<StableDiffusionState> get_sd_state() {
    if (!sd_state) {
        sd_state.reset(new StableDiffusionState());
    }
    return sd_state;
}

StableDiffusionState::StableDiffusionState() {
    generators_.resize(MAX_GENERATORS);
    reloadSdModelList();
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
    }

    return success;
}

const std::list<model_info_t> &StableDiffusionState::getSdModels() const {
    return sdModels_;
}

std::string StableDiffusionState::getSdModelPath(const std::string& name) {
    for (auto it = sdModels_.cbegin(); it != sdModels_.cend(); it++) {
        if (name == it->name) {
            return it->path;
        }
    }
    return "";
}


int StableDiffusionState::randomSeed() {
    int any_random = ((size_t) rand()) % INT32_MAX;
    if (any_random < 10000) 
        any_random += 10000; // we are going to increment and decrement the seeds
    return any_random;
}

generator_cb_t StableDiffusionState::generatorMakeCallback() {
    return [this] (bool success, const char* msg, image_ptr_t result) {
        if (!success || !result) {
            if (msg)
                last_error_ = msg;
            else 
                last_error_ = "Generator failed without error message";
        }
    };
}

void StableDiffusionState::clearGenerators() {
    generators_.clear();
}

void StableDiffusionState::clearImage(int index) {
    if (index >= generators_.size() || index < 0)
        return;
    generators_.erase(generators_.begin() + index);
}

bool StableDiffusionState::generatorAdd(std::shared_ptr<GeneratorBase> generator) {
    last_error_.clear();

    size_t index = 0;
    generator->generate(generatorMakeCallback());

    if (generator->getImage()) {
        generators_.push_back(generator);
    }

    if (generators_.size() > getConfig().getMaxGeneratedImages()) {
        generators_.erase(generators_.begin());
    }

    return last_error_.empty();
}


bool StableDiffusionState::generateNextImage(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    return generatorAdd(generators_[index]->duplicate(false));
}

bool StableDiffusionState::generateNextVariation(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    return generatorAdd(generators_[index]->duplicate(true));
}


image_ptr_t StableDiffusionState::openImage(const char *path) {
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
    } 

    return image;
} 

bool StableDiffusionState::saveImage(const char *path, RawImage *image) {
    if (!image) {
        last_error_ = "no image to save";
        return false;
    }

    last_error_  = std::string();
    bool success = false;
    const char *message = kNO_ERROR_MESSAGE;

    auto cb = dexpert::py::save_image(path, image, [&success, &message] (bool status, const char* msg) {
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


RawImage *StableDiffusionState::getResultsImage(int index) {
    if (index >= generators_.size() || index < 0)
        return NULL;
    return generators_[index]->getImage();
}

const char* StableDiffusionState::lastError() {
    return last_error_.c_str();
}

size_t StableDiffusionState::getGeneratorSize() {
    return generators_.size();
}
    
} // namespace dexpert
