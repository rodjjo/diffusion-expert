/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_CONFIG_CONFIG_H_
#define SRC_CONFIG_CONFIG_H_

#include <string>

namespace dexpert
{

  class Config
  {
  public:
    Config();
    ~Config();

    const std::wstring &executableDir();
    const std::wstring &librariesDir();
    const std::wstring &pyExePath();
    const std::wstring &pythonStuffDir();
    const std::wstring &pythonMainPy();
    const std::wstring &modelsRootDir();
    const std::wstring &sdModelsDir();
    const std::wstring &getConfigDir();

    int screenWidth();
    int screenHeight();

    int windowXPos();
    int windowYPos();
    int windowWidth();
    int windowHeight();

    void setLastSdModel(const std::string &model);
    const char *getLatestSdModel();
    void setSafeFilter(bool enabled);
    bool getSafeFilter();
    void setScheduler(const std::string &name);
    const char *getScheduler();
    int getControlnetCount();
    void setControlnetCount(int value);

    std::string& lastImageSaveDir();
    std::string& lastImageOpenDir();

    bool save();
    bool load();
    
  private:
    // string buffers
    std::wstring configDir_;
    std::wstring librariesDir_;
    std::wstring executableDir_;
    std::wstring pythonStuffDir_;
    std::wstring pyExePath_;
    std::wstring pythonMainPy_;
    std::wstring modelsRootDir_;
    std::wstring sdModelsDir_;

    std::string last_image_open_dir_;
    std::string last_image_save_dir_;

  private:
    // configs
    int controlnetCount_ = 0;
    bool safeFilterEnabled_ = true;
    std::string scheduler_ = "PNDMScheduler";
    std::string lastSdModelName_; // the latest used sd model
  };

  Config &getConfig();

} // namespace dexpert

#endif