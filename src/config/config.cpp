#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <FL/Fl.H>
#include <nlohmann/json.hpp>

#include "src/config/config.h"

using json = nlohmann::json;


namespace dexpert {
namespace {
const wchar_t *kCONFIG_FILE = L"/dexpert.json";
}

Config::Config() {
    load();
}

Config::~Config() {
}

const std::wstring& Config::executableDir() {
    if (executableDir_.empty()) {
#ifdef _WIN32
    wchar_t path[1024] = { 0, };
    if (GetModuleFileNameW(NULL, path, (sizeof(path) / sizeof(wchar_t)) -1) != 0) {
        executableDir_ = path;
    }
#else
    //TODO: create linux implementation
#endif
        size_t latest = executableDir_.find_last_of(L"/\\");
        if (latest != std::wstring::npos) {
            executableDir_ = executableDir_.substr(0, latest);
        } else {
            executableDir_ = std::wstring();
        }
    }

    return executableDir_;
}

const std::wstring& Config::librariesDir() {
    if (librariesDir_.empty()) {
        librariesDir_ = executableDir() + L"/Lib";
        _wmkdir(librariesDir_.c_str());
        librariesDir_ += L"/site-packages";
        _wmkdir(librariesDir_.c_str());
    }
    return librariesDir_;
}

const std::wstring& Config::pythonStuffDir() {
    if (pythonStuffDir_.empty()) {
        pythonStuffDir_ = executableDir() + L"/../python_stuff";
    }
    return pythonStuffDir_;
}

const std::wstring& Config::pythonMainPy() {
    if (pythonStuffDir_.empty()) {
        pythonMainPy_ = pythonStuffDir() + L"/entrypoint.py";
    }
    return pythonMainPy_;
}

const std::wstring& Config::pyExePath() {
    if (pyExePath_.empty()) {
        pyExePath_ = executableDir() + L"/python.exe";
    }
    return pyExePath_;
}

const std::wstring& Config::modelsRootDir() {
    if (modelsRootDir_.empty()) {
        modelsRootDir_ = executableDir() + L"/../models";
        _wmkdir(modelsRootDir_.c_str());
    }
    return modelsRootDir_;
}

const std::wstring& Config::sdModelsDir() {
    if (sdModelsDir_.empty()) {
        sdModelsDir_ = modelsRootDir() + L"/stable-diffusion";
        _wmkdir(sdModelsDir_.c_str());
    }
    return sdModelsDir_;
}

const std::wstring& Config::getConfigDir() {
    if (configDir_.empty()) {
        configDir_ = executableDir() + L"/../config";
        _wmkdir(configDir_.c_str());
    }
    return configDir_;
}

int Config::windowXPos() {
    return screenWidth() / 2 - windowWidth() / 2;
}

int Config::windowYPos() {
    return screenHeight() / 2 - windowHeight() / 2;
}

int Config::windowWidth() {
    return 860;
}

int Config::windowHeight() {
    return 640;
}

int Config::screenWidth() {
    return Fl::w();
}

int Config::screenHeight() {
    return Fl::h();
}

Config &getConfig() {
    static Config cfg;
    return cfg;
}

void Config::setLastSdModel(const std::string& model) {
    lastSdModelName_ = model;
}

const char *Config::getLatestSdModel() {
    return lastSdModelName_.c_str();
}

void Config::setSafeFilter(bool enabled) {
    safeFilterEnabled_ = enabled;
}

bool Config::getSafeFilter() {
    return safeFilterEnabled_;
}

void Config::setScheduler(const std::string &name) {
    scheduler_ = name;
}

const char *Config::getScheduler() {
    return scheduler_.c_str();
}

bool Config::getUseFloat16() {
    return use_float16_;
}

void Config::setUseFloat16(bool value) {
    use_float16_ = value;
}

bool Config::getUseGPU() {
    return use_gpu_;
}

void Config::setUseGPU(bool value) {
    use_gpu_ = value;
}

float Config::gfpgan_get_weight() {
    return gfpgan_weight_;
}

void Config::gfpgan_set_weight(float value) {
    gfpgan_weight_ = value;
}

const char* Config::gfpgan_get_arch() {
    return gfpgan_arch_.c_str();
}

void Config::gfpgan_set_arch(const char *value) {
    gfpgan_arch_ = value;
}

uint32_t Config::gfpgan_get_channel_multiplier() {
    return gfpgan_channel_multiplier_;
}

void Config::gfpgan_set_channel_multiplier(uint32_t value) {
    gfpgan_channel_multiplier_ = value;
}

bool Config::gfpgan_get_only_center_face() {
    return gfpgan_only_center_face_;
}

void Config::gfpgan_set_only_center_face(bool value) {
    gfpgan_only_center_face_ = value;
}

bool Config::gfpgan_get_has_aligned() {
    return gfpgan_has_aligned_;
}

void Config::gfpgan_set_has_aligned(bool value) {
    gfpgan_has_aligned_ = value;
}

bool Config::gfpgan_get_paste_back() {
    return gfpgan_paste_back_;
}

void Config::gfpgan_set_paste_back(bool value) {
    gfpgan_paste_back_ = value;
}

std::string& Config::lastImageSaveDir() {
    return last_image_save_dir_;
}

std::string& Config::lastImageOpenDir() {
    return last_image_open_dir_;
}

float Config::inpaint_get_mask_blur() {
    return inpaint_mask_blur_;
}

void Config::inpaint_set_mask_blur(float value) {
    if (value < 0)
        value = 0;
    if (value > 20) 
        value = 20;

    inpaint_mask_blur_ = value;
}

bool Config::save() {
    try {
        json data;
        json sd;
        sd["last_sd_model"] = lastSdModelName_;
        sd["scheduler"] = scheduler_;
        sd["nsfw_filter"] = safeFilterEnabled_;
        sd["controlnet_count"] = controlnetCount_;
        sd["use_float16"] = use_float16_;
        sd["use_gpu"] = use_gpu_;
        data["stable_diffusion"] = sd;
        json files;
        files["last_image_save_dir"] = last_image_save_dir_;
        files["last_image_open_dir"] = last_image_open_dir_;
        data["files"] = files;
        json gfpgan;
        gfpgan["weight"] = gfpgan_weight_;
        gfpgan["arch"] = gfpgan_arch_;
        gfpgan["channel_multiplier"] = gfpgan_channel_multiplier_;
        gfpgan["only_center_face"] = gfpgan_only_center_face_;
        gfpgan["has_aligned"] = gfpgan_has_aligned_;
        gfpgan["paste_back"] = gfpgan_paste_back_;
        data["gfpgan"] = gfpgan;
        const std::wstring path = getConfigDir() + kCONFIG_FILE;
        std::ofstream f(path.c_str());
        f << std::setw(2) << data << std::endl;
        return true;
    } catch (json::exception& e) {
        fprintf(stderr, "Error saving dexpert configuration file: %s\n", e.what());
    }

    return false;
}

bool Config::load() {
    const std::wstring path = getConfigDir() + kCONFIG_FILE;
    std::ifstream f(path.c_str());
    try {
        if (!f.good()) {
            fprintf(stderr, "Diffusion Expert's configuration file does not exist\n");
            return true;
        }
        json data = json::parse(f);
        if (data.contains("stable_diffusion")) {
            auto sd = data["stable_diffusion"];
            if (sd.contains("last_sd_model")) {
                lastSdModelName_ = sd["last_sd_model"].get<std::string>();
            }
            if (sd.contains("scheduler")) {
                scheduler_ = sd["scheduler"].get<std::string>();
            }
            if (sd.contains("nsfw_filter")) {
                safeFilterEnabled_ = sd["nsfw_filter"].get<bool>();
            }
            if (sd.contains("controlnet_count")) {
                controlnetCount_ = sd["controlnet_count"].get<int>();
                if (controlnetCount_ < 0) {
                    controlnetCount_ = 0;
                }
                if (controlnetCount_ > 4) {
                    controlnetCount_ = 4;
                }
            }
            if (sd.contains("use_float16")) {
                use_float16_ = sd["use_float16"].get<bool>();
            }
            if (sd.contains("use_gpu")) {
                use_gpu_ = sd["use_gpu"].get<bool>();
            }
        }
        if (data.contains("files")) {
            auto files = data["files"];
            if (files.contains("last_image_save_dir")) {
                last_image_save_dir_ = files["last_image_save_dir"].get<std::string>();
            }
            if (files.contains("last_image_open_dir")) {
                last_image_open_dir_ = files["last_image_open_dir"].get<std::string>();
            }
        }
        if (data.contains("gfpgan")) {
            auto gfpgan = data["gfpgan"];
            if (gfpgan.contains("weight")) {
                gfpgan_weight_ = gfpgan["weight"].get<float>();
                if (gfpgan_weight_ > 1.0)
                    gfpgan_weight_ = 1.0;
                else if (gfpgan_weight_ < 0.0)
                    gfpgan_weight_ = 0;
            }
            if (gfpgan.contains("arch")) {
                gfpgan_arch_ = gfpgan["arch"].get<std::string>();
            }
            if (gfpgan.contains("channel_multiplier")) {
                gfpgan_channel_multiplier_ = gfpgan["channel_multiplier"].get<uint32_t>();
            }
            if (gfpgan.contains("only_center_face")) {
                gfpgan_only_center_face_ = gfpgan["only_center_face"].get<bool>();
            }
            if (gfpgan.contains("has_aligned")) {
                gfpgan_has_aligned_ = gfpgan["has_aligned"].get<bool>();
            }
            if (gfpgan.contains("paste_back")) {
                gfpgan_paste_back_ = gfpgan["paste_back"].get<bool>();
            }
        }
        return true;
    } catch(json::exception& e) {
        fprintf(stderr, "Error loading dexpert configuration file: %s\n", e.what());
    }

    return false;
}

int Config::getControlnetCount() {
    return controlnetCount_;
}

void Config::setControlnetCount(int value) {
    controlnetCount_ = value;
}

}  // namespace dexpert
