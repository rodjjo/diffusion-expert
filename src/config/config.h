/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_CONFIG_CONFIG_H_
#define SRC_CONFIG_CONFIG_H_

#include <string>

namespace dexpert {

class Config {
 public:
   Config();
   ~Config();

   const std::wstring& executableDir();
   const std::wstring& librariesDir();
   const std::wstring& pyExePath();
   const std::wstring& pythonStuffDir();
   const std::wstring& pythonMainPy();
   const std::wstring& modelsRootDir();
   const std::wstring& sdModelsDir();

   int screenWidth();
   int screenHeight();
   
   int windowXPos();
   int windowYPos();
   int windowWidth();
   int windowHeight();

 private:
   std::wstring librariesDir_;
   std::wstring executableDir_;
   std::wstring pythonStuffDir_;
   std::wstring pyExePath_;
   std::wstring pythonMainPy_;
   std::wstring modelsRootDir_;
   std::wstring sdModelsDir_;
};

Config &getConfig();

}  // namespace dexpert

#endif