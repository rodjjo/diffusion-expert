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
                printf("model: %s %s\n", it->name.c_str(), it->path.c_str());
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
        setSdModel(getConfig().getLatestSdModel());
        if (currentSdModel_.empty()) {
            setSdModel(sdModels_.begin()->name);
        }
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

std::string StableDiffusionState::getSdModelPath(const std::string& name) {
    for (auto it = sdModels_.cbegin(); it != sdModels_.cend(); it++) {
        if (name == it->name) {
            return it->path;
        }
    }
    return "";
}

void StableDiffusionState::scroll_down_generators() {
    for (size_t i = generators_.size(); i > 1; --i) {
        if ((int)i - 1 > 0) {
            generators_[i - 1] = generators_[i - 2];
        }
    }
    generators_.begin()->g.reset();
}

void StableDiffusionState::scroll_up_generators() {
    for (size_t i = 0; i < generators_.size(); ++i) {
        if (i + 1 < generators_.size()) {
            generators_[i] = generators_[i + 1];
        }
    }
    generators_.rbegin()->g.reset();
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
    for (size_t i = 0; i < generators_.size(); ++i) {
        generators_[i].g.reset();
        generators_[i].seed_increment = 0;
    }
}

void StableDiffusionState::clearImage(int index, int variation) {
    if (index >= generators_.size() || index < 0)
        return;
    if (!generators_[index].g) 
        return;
    if (variation == 0) {
        return generators_[index].g->clearImage();
    }

    return generators_[index].g->clearVariation(variation - 1);
}

bool StableDiffusionState::generatorAdd(std::shared_ptr<GeneratorBase> generator) {
    last_error_.clear();

    clearGenerators();

    size_t index = 0;
    generator->generate(generatorMakeCallback(), 0);

    if (generator->getImage()) {
        generators_[index].g = generator;
        generators_[index].seed_increment = 0;
    }

    return last_error_.empty();
}

bool StableDiffusionState::generatePreviousImage(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    auto g = generators_[index];
    if (!g.g) {
        last_error_ = "No Generator";
        return false;
    }

    last_error_.clear();
    g.g = g.g->duplicate();
    g.seed_increment -= 1;

    g.g->generate(generatorMakeCallback(), g.seed_increment, 0);
    
    if (g.g->getImage()) {
        if (generators_[0].g) {
            scroll_down_generators();
        }
        generators_[0] = g;
    }

    return last_error_.empty();
}

bool StableDiffusionState::generateNextImage(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    auto g = generators_[index];
    if (!g.g) {
        last_error_ = "No Generator";
        return false;
    }
    last_error_.clear();
    
    index = generators_.size() - 1;
    bool scroll_up = false;
    if (generators_[index].g) {
        scroll_up = true;
    } else {
        while (index > 0) {
            if (generators_[index - 1].g) {
                break;
            }
            --index;
        }
    }
    
    g.g = g.g->duplicate();
    g.seed_increment += 1;
    
    g.g->generate(generatorMakeCallback(), g.seed_increment, 0);

    if (g.g->getImage()) {
        if (scroll_up)
            scroll_up_generators();
        generators_[index] = g;
    }

    return last_error_.empty();
}

bool StableDiffusionState::generatePreviousVariation(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    if (!generators_[index].g) 
        return false;
    last_error_ = "";
    auto &g = generators_[index];
    g.g->generate(generatorMakeCallback(), g.seed_increment, -1);
    return last_error_.empty();
}

bool StableDiffusionState::generateNextVariation(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= generators_.size())
        return false;
    if (!generators_[index].g) 
        return false;
    last_error_ = "";
    auto &g = generators_[index];
    g.g->generate(generatorMakeCallback(), g.seed_increment, 1);
    return last_error_.empty();
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


RawImage *StableDiffusionState::getResultsImage(int index, int variation) {
    if (index >= generators_.size() || index < 0)
        return NULL;
    if (!generators_[index].g) 
        return NULL;
    if (variation == 0) {
        return generators_[index].g->getImage();
    }
    return generators_[index].g->getVariation(variation - 1);
}

RawImage *StableDiffusionState::getControlNetImage(int index) {
    return NULL;
}

int StableDiffusionState::getMaxResultImages() {
    return generators_.size();
}

int StableDiffusionState::getMaxResultVariations() {
    return  GeneratorBase::maxVariations() + 1; // +1 of original image
}

const char* StableDiffusionState::lastError() {
    return last_error_.c_str();
}
    
} // namespace dexpert
