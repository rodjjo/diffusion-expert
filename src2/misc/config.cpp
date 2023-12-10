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

            auto load_key = [&cf] (const char *key, const char *sub_key) -> std::string {
                std::string r;
                if (!cf.contains(key)) {
                    return r;
                }
                auto sub = py11::cast<py11::dict>(cf[key]);
                if (sub.contains(sub_key)) {
                    return py11::cast<std::string>(sub[sub_key]);
                }
                return r;
            };

            add_lora_dir_ = load_key("directories", "add_lora_dir");
            add_emb_dir_ = load_key("directories", "add_emb_dir");
            add_model_dir_ = load_key("directories", "add_model_dir");

            if (cf.contains("nsfw_filter_enabled")) {
                filter_nsfw_ = py11::cast<bool>(cf["nsfw_filter_enabled"]);
            }
            if (cf.contains("float16_enabled")) {
                use_float16_ = py11::cast<bool>(cf["float16_enabled"]);
            }
            if (cf.contains("private_mode_enabled")) {
                private_mode_ = py11::cast<bool>(cf["private_mode_enabled"]);
            }
            if (cf.contains("keep_in_memory")) {
                keep_in_memory_ = py11::cast<bool>(cf["keep_in_memory"]);
            }
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
            
            py11::dict dirs;
            dirs["add_lora_dir"] = add_lora_dir_;
            dirs["add_emb_dir"] = add_emb_dir_;
            dirs["add_model_dir"] = add_model_dir_;
            cf["directories"] = dirs;
            
            cf["nsfw_filter_enabled"] = filter_nsfw_;
            cf["float16_enabled"] = use_float16_;
            cf["private_mode_enabled"] = private_mode_;
            cf["keep_in_memory"] = keep_in_memory_;
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

    std::string Config::add_model_dir() {
        return add_model_dir_;
    }

    std::string Config::add_lora_dir() {
        return add_lora_dir_;
    }

    std::string Config::add_emb_dir() {
        return add_emb_dir_;
    }

    void Config::add_model_dir(const char *value) {
        add_model_dir_ = value;
    }

    void Config::add_lora_dir(const char *value) {
        add_lora_dir_ = value;
    }

    void Config::add_emb_dir(const char *value) {
        add_emb_dir_ = value;
    }

    bool Config::filter_nsfw() {
        return filter_nsfw_;
    }

    bool Config::use_float16() {
        return use_float16_;
    }

    bool Config::private_mode() {
        return private_mode_;
    }

    void Config::filter_nsfw(bool value) {
        filter_nsfw_ = value;
    }

    void Config::use_float16(bool value) {
        use_float16_ = value;
    }

    void Config::private_mode(bool value) {
        private_mode_ = value;
    }

    void Config::keep_in_memory(bool value) {
        keep_in_memory_ = value;
    }

    bool Config::keep_in_memory() {
        return keep_in_memory_;
    }

} // namespace dfe
