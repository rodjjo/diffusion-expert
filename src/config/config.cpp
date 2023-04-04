#ifdef _WIN32
#include <Windows.h>
#endif

#include <FL/Fl.H>
#include "src/config/config.h"

namespace dexpert {

Config::Config() {
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
        librariesDir_ = executableDir() + L"\\Lib";
        _wmkdir(librariesDir_.c_str());
        librariesDir_ += L"\\site-packages";
        _wmkdir(librariesDir_.c_str());
    }
    return librariesDir_;
}

const std::wstring& Config::pythonStuffDir() {
    if (pythonStuffDir_.empty()) {
        pythonStuffDir_ = executableDir() + L"\\..\\python_stuff";
    }
    return pythonStuffDir_;
}

const std::wstring& Config::pythonMainPy() {
    if (pythonStuffDir_.empty()) {
        pythonMainPy_ = pythonStuffDir() + L"\\main.py";
    }
    return pythonMainPy_;
}

const std::wstring& Config::pyExePath() {
    if (pyExePath_.empty()) {
        pyExePath_ = executableDir() + L"\\python.exe";
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

int Config::windowXPos() {
    return screenWidth() / 2 - windowWidth() / 2;
}

int Config::windowYPos() {
    return screenHeight() / 2 - windowHeight() / 2;
}

int Config::windowWidth() {
    return 640;
}

int Config::windowHeight() {
    return 480;
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

}  // namespace dexpert
