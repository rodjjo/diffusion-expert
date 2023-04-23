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
    bool getUseFloat16();
    bool getUseGPU();
    void setUseFloat16(bool value);
    void setUseGPU(bool value);
    float gfpgan_get_weight();
    void gfpgan_set_weight(float value);
    const char* gfpgan_get_arch();
    void gfpgan_set_arch(const char *value);
    uint32_t gfpgan_get_channel_multiplier();
    void gfpgan_set_channel_multiplier(uint32_t value);
    bool gfpgan_get_only_center_face();
    void gfpgan_set_only_center_face(bool value);
    bool gfpgan_get_has_aligned();
    void gfpgan_set_has_aligned(bool value);
    bool gfpgan_get_paste_back();
    void gfpgan_set_paste_back(bool value);

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
    bool use_gpu_ = true;
    bool use_float16_ = true;
    float gfpgan_weight_ = 0.5;
    std::string gfpgan_arch_ = "clean";
    uint32_t gfpgan_channel_multiplier_ = 2;
    bool gfpgan_only_center_face_ = false;
    bool gfpgan_has_aligned_ = false;
    bool gfpgan_paste_back_ = true;
    int controlnetCount_ = 0;
    bool safeFilterEnabled_ = true;
    std::string scheduler_ = "PNDMScheduler";
    std::string lastSdModelName_; // the latest used sd model
  };

  Config &getConfig();

} // namespace dexpert

#endif