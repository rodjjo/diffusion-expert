#include <memory>
#include <string>
#include <map>

namespace dfe
{

class Config {
    public:
        Config();
        virtual ~Config();
        bool load();
        bool save();
        std::string last_save_directory(const char *scope);
        std::string last_open_directory(const char *scope);
        void last_save_directory(const char *scope, const char* value);
        void last_open_directory(const char *scope, const char* value);
        std::string add_model_dir();
        std::string add_lora_dir();
        std::string add_emb_dir();
        void add_model_dir(const char *value);
        void add_lora_dir(const char *value);
        void add_emb_dir(const char *value);

        bool filter_nsfw();
        bool use_float16();
        bool private_mode();
        bool keep_in_memory();

        void filter_nsfw(bool value);
        void use_float16(bool value);
        void private_mode(bool value);
        void keep_in_memory(bool value);

    private:
        std::map<std::string, std::string> last_save_dirs;
        std::map<std::string, std::string> last_open_dirs;
        std::string add_model_dir_;
        std::string add_lora_dir_;
        std::string add_emb_dir_;
        bool filter_nsfw_ = true;
        bool use_float16_ = true;
        bool private_mode_ = false;
        bool keep_in_memory_ = false;
};
    
Config* get_config();

} // namespace dfe
