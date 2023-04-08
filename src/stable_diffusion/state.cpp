#include "src/stable_diffusion/state.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"

namespace dexpert {
namespace {

const char *kNO_ERROR_MESSAGE = "Error with no error message";
const int kMAX_RESULT_IMAGES = 4;
const int kMAX_RESULT_VARIATIONS = 4;

std::shared_ptr<StableDiffusionState> sd_state;

}  // unnamed namespace

std::shared_ptr<StableDiffusionState> get_sd_state() {
    if (!sd_state) {
        sd_state.reset(new StableDiffusionState());
    }
    return sd_state;
}

StableDiffusionState::StableDiffusionState() {
    for (int r = 0; r < kMAX_RESULT_IMAGES; ++r) {
        image_list_t row;
        row.push_back(image_ptr_t());
        for (int v = 0; v <kMAX_RESULT_VARIATIONS; ++v)  {
            row.push_back(image_ptr_t());
        }
        result_images_.push_back(row);
    }
    generators_.resize(kMAX_RESULT_IMAGES);
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

std::wstring StableDiffusionState::getSdModelPath(const std::string& name) {
    for (auto it = sdModels_.cbegin(); it != sdModels_.cend(); it++) {
        if (name == it->name) {
            return it->path;
        }
    }
    return L"";
}

void StableDiffusionState::scroll_down_generators() {
    for (size_t i = generators_.size(); i > 0; --i) {
        if ((int)i - 1 >= 0) {
            generators_[i] = generators_[i - 1];
            result_images_[i] = result_images_[i - 1];
        }
    }
    generators_.begin()->first.reset();
    generators_.begin()->second = 0;
    for (size_t i = 0; i < result_images_.begin()->size(); ++i) {
        result_images_.begin()->at(i).reset();
    }
}

void StableDiffusionState::scroll_up_generators() {
    for (size_t i = 0; i < generators_.size(); ++i) {
        if (i + 1 < generators_.size()) {
            generators_[i] = generators_[i + 1];
            result_images_[i] = result_images_[i + 1];
        }
    }
    generators_.rbegin()->first.reset();
    generators_.rbegin()->second = 0;
    for (size_t i = 0; i < result_images_.rbegin()->size(); ++i) {
        result_images_.rbegin()->at(i).reset();
    }
}

int StableDiffusionState::randomSeed() {
    int any_random = ((size_t) rand()) % INT32_MAX;
    if (any_random < 10000) 
        any_random += 10000; // we are going to increment and decrement the seeds
    return any_random;
}

generator_cb_t StableDiffusionState::generatorMakeCallback(int index, int variation) {
    return [this, index, variation] (bool success, const char* msg, image_ptr_t result) {
        if (!success || !result) {
            if (msg)
                last_error_ = msg;
            else 
                last_error_ = "Generator failed without error message";
        } else {
            result_images_[index][variation] = result;
        }
    };
}

bool StableDiffusionState::generatorAdd(std::shared_ptr<GeneratorBase> generator) {
    last_error_.clear();

    size_t index = 0;
    for (size_t i = 0; i < generators_.size(); ++i) {
        index = i;
        if (!generators_[i].first) {
            break;
        }
    }
    if (generators_[index].first) {
        scroll_up_generators();
    }
    generators_[index] = std::make_pair(generator, 0);
    generator->generate(generatorMakeCallback(index, 0), 0, 0, 0);

    return last_error_.empty();
}

bool StableDiffusionState::generatePreviousImage(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= result_images_.size())
        return false;
    auto g = generators_[index];
    if (!g.first) {
        last_error_ = "No Generator";
        return false;
    }

    last_error_.clear();

    if (generators_[0].first) {
        scroll_down_generators();
    }
    g.second -= 1;
    generators_[0] = g;
    g.first->generate(generatorMakeCallback(0, 0), g.second, 0, 0);

    return last_error_.empty();
}

bool StableDiffusionState::generateNextImage(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= result_images_.size())
        return false;
    auto g = generators_[index];
    if (!g.first) {
        last_error_ = "No Generator";
        return false;
    }
    last_error_.clear();
    
    index = generators_.size() - 1;
    if (generators_[index].first) {
        scroll_up_generators();
    } else {
        while (index > 0) {
            if (generators_[index - 1].first) {
                break;
            }
            --index;
        }
    }
    
    g.second += 1;
    generators_[index] = g;
    g.first->generate(generatorMakeCallback(index, 0), g.second, 0, 0);

    return last_error_.empty();
}

void StableDiffusionState::scroll_left(int index) {
    if (index < 0 || index >= result_images_.size())
        return;
    auto &row = result_images_[index];
    for (size_t i = 1; i < row.size(); ++i) {
        if (i + 1 < row.size()) {
            row[i] = row[i + 1];
        }
    }
    row.rbegin()->reset();
}

void StableDiffusionState::scroll_right(int index) {
    if (index < 0 || index >= result_images_.size())
        return;
    auto &row = result_images_[index];
    for (size_t i = row.size(); i > 1; --i) {
        if (i - 1 >= 0) {
            row[i] = row[i - 1];
        }
    }
    row[1].reset();
}

bool StableDiffusionState::generatePreviousVariation(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= result_images_.size())
        return false;
    last_error_ = "";
    if (result_images_[index][1])
        scroll_right(index);
    auto &row = result_images_[index];
    auto &g = generators_[index];
    int variation = getMinVariation(index);
    g.first->generate(generatorMakeCallback(index, 1), g.second, variation, 0.3, true);
    return last_error_.empty();
}

bool StableDiffusionState::generateNextVariation(int index) {
    last_error_ = "Wrong index";
    if (index < 0 || index >= result_images_.size())
        return false;
    last_error_ = "";
    if (result_images_[index].rbegin()->get())
        scroll_left(index);
    auto &row = result_images_[index];
    int store_var = row.size() - 1;
    for (size_t i = 1; i < row.size(); ++i) {
        if (!row[i]) {
            store_var = i;
            break;
        }
    }
    auto &g = generators_[index];
    int variation = getMaxVariation(index);
    g.first->generate(generatorMakeCallback(index, store_var), g.second, variation, 0.3, true);
    return last_error_.empty();
}

int StableDiffusionState::getMinVariation(int index) {
    if (index < 0 || index >= result_images_.size())
        return 0;
    auto &row = result_images_[index];
    for (size_t i = 1; i < row.size(); ++i) {
        if (row[i]) {
            return row[i]->getVariation();
        }
    }
    return 0;
}

int StableDiffusionState::getMaxVariation(int index) {
    if (index < 0 || index >= result_images_.size())
        return 0;
    auto &row = result_images_[index];
    for (size_t i = row.size(); i > 1; --i) {
        if (row[i - 1]) {
            return row[i]->getVariation();
        }
    }
    return 0;
}

/*
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
} */

/*
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
*/

RawImage *StableDiffusionState::getResultsImage(int index, int variation) {
    if (index >= result_images_.size())
        return NULL;
    if (variation >= result_images_[index].size())
        return NULL;
    return result_images_[index][variation].get();
}

RawImage *StableDiffusionState::getControlNetImage(int index) {
    return NULL;
}

int StableDiffusionState::getMaxResultImages() {
    return result_images_.size();
}

int StableDiffusionState::getMaxResultVariations() {
    return result_images_[0].size();
}

void StableDiffusionState::clearAllResultImages() {
    for (int i = 0; i < result_images_.size(); ++i) {
        auto &v = result_images_[i];
        for (int j = 0; j < v.size(); ++j) {
            v[j].reset();
        }
    }
}

void StableDiffusionState::clearResult(int index, int variation) {
    if (index >= result_images_.size())
        return;
    if (variation >= result_images_[index].size())
        return;
    result_images_[index][variation].reset();
}

const char* StableDiffusionState::lastError() {
    return last_error_.c_str();
}
    
} // namespace dexpert
