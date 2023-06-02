#include "utils.h"

#include "src/config/config.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"


namespace dexpert
{


image_ptr_t open_image_from_dialog() {
    image_ptr_t result;
    std::string path = choose_image_to_open(&getConfig().lastImageOpenDir());
    if (!path.empty()) {
        result = get_sd_state()->openImage(path.c_str());
        if (!result) {
            show_error(get_sd_state()->lastError());
        } else {
            getConfig().save();
        }
    }
    return result;
}

bool save_image_with_dialog(image_ptr_t img) {
    std::string path = choose_image_to_save(&getConfig().lastImageSaveDir());
    if (!path.empty()) {
        if (!get_sd_state()->saveImage(path.c_str(), img.get())) {
            show_error(get_sd_state()->lastError());
        } else {
            getConfig().save();
        }
    }
    return false;
}


} // namespace dexpert
