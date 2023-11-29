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
    private:
        std::map<std::string, std::string> last_save_dirs;
        std::map<std::string, std::string> last_open_dirs;

};
    
Config* get_config();

} // namespace dfe
