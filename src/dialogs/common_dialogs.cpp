
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <Fl/fl_ask.H>

#ifdef _WIN32
#include <Windows.h>
#endif 

#include "src/config/config.h"
#include "src/dialogs/common_dialogs.h"

namespace dexpert
{
namespace {
    const char *kIMAGE_FILES_FILTER = "Image files\t*.{png,bmp,jpeg,webp,gif,jpg}\n";
    const char *kIMAGE_FILES_FILTER_FL = "Image files (*.{png,bmp,jpeg,webp,gif,jpg})";
}

bool ask(const char *message) {
    return fl_choice("%s", "No", "Yes", NULL, message) == 1;
}

response_t yes_nc(const char *message) {
    int r = fl_choice("%s", "No", "Cancel", "Yes", message);
    switch (r)
    {
    case 0:
        return r_yes;
        break;
    case 1:
        return r_cancel;
        break;
    default:
        return r_no;
    }
}

void show_error(const char *message) {
    fl_alert("%s", message);
}

const char *ask_value(const char *message) {
    return fl_input("%s", "", message);
}

bool path_exists(const char *p) {
#ifdef _WIN32
    return GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES;
#else 
    return false;
#endif
}

std::string execute_file_choose(Fl_Native_File_Chooser *fc, std::string* current_dir, const char *default_extension) {
    std::string path;

    if (current_dir && !current_dir->empty()) {
        if (path_exists(current_dir->c_str())) {
            fc->directory(current_dir->c_str());
        }
    }

    if (fc->show() == 0) {
        path = fc->filename();

        if (current_dir) {
            std::string dir = std::string(path);
            size_t latest = dir.find_last_of("/\\");
            if (latest != std::wstring::npos) {
                dir = dir.substr(0, latest);
            } else {
                dir = std::string();
            }
            if (path_exists(dir.c_str())) {
                *current_dir = dir;
            }
        }

        uint32_t extension_lenght = default_extension ? strlen(default_extension) - 1 : 3;

        if (default_extension && (path.size() <= extension_lenght || *(path.rbegin() + extension_lenght) != '.')) {
            path += default_extension;
        }
    }

    return path;
}

void configure_chooser(Fl_Native_File_Chooser *dialog, const char *filter, const char *title, bool saving=true) {
    if (saving) {
        dialog->options(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    }
    dialog->title(title);
    dialog->filter(filter);
}

std::string choose_image_to_open(std::string* current_dir) {
    if (dexpert::getConfig().getPrivacyMode()) {
        return choose_image_to_open_fl(current_dir);
    }
    Fl_Native_File_Chooser dialog(Fl_Native_File_Chooser::BROWSE_FILE);
    configure_chooser(&dialog, kIMAGE_FILES_FILTER, "Select an image to open", false);
    return execute_file_choose(&dialog, current_dir, NULL);
}

std::string choose_image_to_save(std::string* current_dir) {
    if (dexpert::getConfig().getPrivacyMode()) {
        return choose_image_to_save_fl(current_dir);
    }
    Fl_Native_File_Chooser dialog(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    configure_chooser(&dialog, kIMAGE_FILES_FILTER, "Save the image", true);
    std::string result = execute_file_choose(&dialog, current_dir, ".png");
    if (!result.empty() && path_exists(result.c_str())) {
        if (!ask("Do you want to replace the destination file ?")) {
            result.clear();
        }
    }
    return result;
}

std::string executeChooser(Fl_File_Chooser *fc) {
    fc->preview(0);
    fc->show();
    while (fc->shown()) {
        Fl::wait(0.01);
    }
    if (fc->value()) return fc->value();
    return std::string();
}

std::string choose_image_to_open_fl(std::string* current_dir) {
    if (!path_exists(current_dir->c_str())) {
        *current_dir = "";
    }
    Fl_File_Chooser dialog(current_dir->c_str(), kIMAGE_FILES_FILTER_FL, Fl_File_Chooser::SINGLE, "Open image");
    std::string result = executeChooser(&dialog);
    if (!result.empty()) {
        size_t latest = result.find_last_of("/\\");
        *current_dir = result.substr(0, latest);
    }
    return result;
}

std::string choose_image_to_save_fl(std::string* current_dir) {
    if (!path_exists(current_dir->c_str())) {
        *current_dir = "";
    }
    Fl_File_Chooser dialog(current_dir->c_str(), kIMAGE_FILES_FILTER_FL, Fl_File_Chooser::SINGLE | Fl_File_Chooser::CREATE, "Save image");
    std::string result = executeChooser(&dialog);
    
    if (!result.empty() && path_exists(result.c_str())) {
        if (!ask("Do you want to replace the destination file ?")) {
            result.clear();
        } else {
            size_t latest = result.find_last_of("/\\");
            *current_dir = result.substr(0, latest);
        }
    }

    return result;
}


bool pickup_color(const char* title, uint8_t *r, uint8_t *g, uint8_t *b) {
    return fl_color_chooser(title, *r, *g, *b) == 1;
}

} // namespace dexpert
