#include "misc/config.h"
#include "python/routines.h"

namespace dfe
{
    namespace 
    {
        std::unique_ptr<Config> conf;
    } // namespace 
    
    Config* get_config() {
        if (!conf) {
            conf.reset(new Config());
            conf->load();
        }
        return conf.get();
    }

    Config::Config() {
    }

    Config::~Config() {
    }

    bool Config::load() {
        auto cf = py::load_config();
        if (cf.empty()) {
            puts("Configuration empty, using the default values...");
            return true; // there is no config saved yet use the defaults
        }
        try {
            auto load_map = [&cf] (const char *key, std::map<std::string, std::string> & output) {
                if (cf.contains(key)) {
                    auto hist = py11::cast<py11::dict>(cf[key]);
                    output.clear();
                    for (const auto & v: hist) {
                        output[py11::cast<std::string>(v.first)] = py11::cast<std::string>(v.second);
                    }
                }
            };
            load_map("open_history", last_open_dirs);
            load_map("save_history", last_save_dirs);
            
        } catch(std::exception e) {
            printf("Failed to load the config: %s", e.what());
            return false;
        }
        return true;
    }

    bool Config::save() {
        py11::dict cf;
        try {
            auto store_map = [&cf] (const char *key, const std::map<std::string, std::string> & input) {
                py11::dict d;
                for (const auto & v: input) {
                    d[v.first.c_str()] = v.second;
                }
                cf[key] = d;
            };
            store_map("open_history", last_open_dirs);
            store_map("save_history", last_save_dirs);
        } catch(std::exception e) {
            printf("Failed to save the config: %s", e.what());
            return false;
        }
        py::store_config(cf);
        return true;
    }

    std::string Config::last_save_directory(const char *scope) {
        auto it = last_open_dirs.find(scope);
        if (last_save_dirs.end() != it) {
            return it->second;
        }
        return std::string();
    }

    std::string Config::last_open_directory(const char *scope) {
        auto it = last_open_dirs.find(scope);
        if (last_open_dirs.end() != it) {
            return it->second;
        }
        return std::string();
    }

    void Config::last_save_directory(const char *scope, const char* value) {
        last_save_dirs[scope] = value;
        save();
    }

    void Config::last_open_directory(const char *scope, const char* value) {
        last_open_dirs[scope] = value;
        save();
    }
} // namespace dfe
