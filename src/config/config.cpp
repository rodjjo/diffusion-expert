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
        pythonMainPy_ = pythonStuffDir() + L"/main.py";
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
    return 720;
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
std::string& Config::lastImageSaveDir() {
    return last_image_save_dir_;
}

std::string& Config::lastImageOpenDir() {
    return last_image_open_dir_;
}

bool Config::save() {
    try {
        json data;
        json sd;
        sd["last_sd_model"] = lastSdModelName_;
        sd["scheduler"] = scheduler_;
        sd["nsfw_filter"] = safeFilterEnabled_;
        sd["controlnet_count"] = controlnetCount_;
        data["stable_diffusion"] = sd;
        json files;
        files["last_image_save_dir"] = last_image_save_dir_;
        files["last_image_open_dir"] = last_image_open_dir_;
        data["files"] = files;
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
