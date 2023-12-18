
#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <Fl/fl_ask.H>

#ifdef _WIN32
#include <Windows.h>
#endif 

#include "misc/config.h"
#include "misc/dialogs.h"

namespace dfe
{
    
namespace {
    const char *kIMAGE_FILES_FILTER = "Image files\t*.{png,bmp,jpeg,webp,gif,jpg}\n";
    const char *kIMAGE_FILES_FILTER_FL = "Image files (*.{png,bmp,jpeg,webp,gif,jpg})";
}

bool path_exists(const char *p) {
#ifdef _WIN32
    return GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES;
#else 
    return false;
#endif
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

std::string executeChooser(Fl_File_Chooser *fc) {
    fc->preview(0);
    fc->show();
    fc->position(Fl::w() / 2 - fc->w() / 2, Fl::h() / 2 - fc->h() / 2);
    while (fc->shown()) {
        Fl::wait(0.01);
    }
    if (fc->value()) return fc->value();
    return std::string();
}

std::string choose_image_to_open_fl(const std::string& scope) {
    std::string current_dir = get_config()->last_open_directory(scope.c_str());

    if (!path_exists(current_dir.c_str())) {
        current_dir = "";
    }
    Fl_File_Chooser dialog(current_dir.c_str(), kIMAGE_FILES_FILTER_FL, Fl_File_Chooser::SINGLE, "Open image");
    std::string result = executeChooser(&dialog);
    if (!result.empty()) {
        size_t latest = result.find_last_of("/\\");
        current_dir = result.substr(0, latest);
        get_config()->last_open_directory(scope.c_str(), current_dir.c_str());
    }
    return result;
}

std::string choose_image_to_save_fl(const std::string& scope) {
    std::string current_dir = get_config()->last_save_directory(scope.c_str());
    if (!path_exists(current_dir.c_str())) {
        current_dir = "";
    }
    Fl_File_Chooser dialog(current_dir.c_str(), kIMAGE_FILES_FILTER_FL, Fl_File_Chooser::SINGLE | Fl_File_Chooser::CREATE, "Save image");
    std::string result = executeChooser(&dialog);
    
    if (!result.empty() && path_exists(result.c_str())) {
        if (!ask("Do you want to replace the destination file ?")) {
            result.clear();
        } else {
            size_t latest = result.find_last_of("/\\");
            current_dir = result.substr(0, latest);
            get_config()->last_save_directory(scope.c_str(), current_dir.c_str());
        }
    }

    return result;
}


} // namespace dfe
