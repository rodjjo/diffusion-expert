#include <thread>
#include <string>
#include <FL/fl_ask.H>

#include "python/routines.h"
#include "windows/progress_ui.h"

namespace dfe
{
namespace py {

std::string parse_dict_error(py11::dict &dict)
{
    std::string result;
    if (dict.contains("error"))
    {
        try {
            result = dict["error"].cast<std::string>();
        } catch (std::exception e) {
            result = std::string("Error casting error result from python: ") + e.what();
        }
    }
    return result;
}

image_ptr_t open_image(const char* path) {
    image_ptr_t result;
    std::string error;

    execute([&result, &error, path] (py11::module_ &module) {
        try {
            auto r = module.attr("open_image")(path);
            py11::dict d = r.cast<py11::dict>();
            error = parse_dict_error(d);
            if (error.empty()) {
                result = dfe::py::rawImageFromPyDict(d);
                if (!result) {
                    error = "Could not parse the dictionary";
                }
            }
        } catch(std::exception e) {
            error = e.what();
        }
    });

    if (!result.get()) {
        fl_alert("Error opening the image: %s", error.c_str());
    } 
    return result;
}

image_ptr_t remove_background(RawImage* img, const py11::dict& params) {
    image_ptr_t result;
    std::string error;
    
    enable_progress_window(progress_background);

    execute([&result, &error, img, params] (py11::module_ &module) {
        try {
            py11::dict src;
            img->toPyDict(src);
            auto r = module.attr("remove_background")(src, params);
            py11::dict d = r.cast<py11::dict>();
            error = parse_dict_error(d);
            if (error.empty()) {
                result = dfe::py::rawImageFromPyDict(d);
                if (!result) {
                    error = "Could not parse the dictionary";
                }
            }
        } catch(std::exception e) {
            error = e.what();
        }
    });
    
    hide_progress_window();

    if (!result.get()) {
        fl_alert("Error removing the background of the image: %s", error.c_str());
    } 
    return result;
}

py11::dict load_config() {
    py11::dict result;
    std::string error;
    execute([&result, &error] (py11::module_ &module) {
        try {
            auto r = module.attr("load_settings")();
            result = r.cast<py11::dict>();
        } catch(std::exception e) {
            error = e.what();
        }
    });
    if (!error.empty()) {
        fl_alert("Error loading the config: %s", error.c_str());
    } 
    return result;
}

void store_config(const py11::dict& config) {
    std::string error;
    execute([&config, &error] (py11::module_ &module) {
        try {
            module.attr("store_settings")(config);
        } catch(std::exception e) {
            error = e.what();
        }
    });
    if (!error.empty()) {
        fl_alert("Error loading the config: %s", error.c_str());
    } 
}


}
} // namespace dfe
