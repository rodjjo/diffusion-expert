#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/panels/frame_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"

namespace dexpert {

FramePanel::FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h): OpenGlPanel(x, y, w, h) {

}

FramePanel::~FramePanel() {

}

bool FramePanel::openFile() {
    std::string path = choose_image_to_open(&current_open_dir_);
    if (!path.empty()) {
        bool success = false;
        const char *message = "Unexpected error. Callback to open image not called";
        std::shared_ptr<dexpert::py::RawImage> image;
        auto cb = dexpert::py::open_image(path.c_str(), [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
            success = status;
            message = msg;
            image = img;
        });
        dexpert::py::get_py()->execute_callback(cb);
        if (!success) {
            show_error(message);
        } else if (image) {
            image_ = image;
            redraw();
        }
        return success;
    }
    
    return false;
}

void FramePanel::get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) {
    if (image_) {
        *buffer = image_->buffer();
        *w = image_->w();
        *h = image_->h();
        *format = GL_LUMINANCE;
        if (image_->format() == dexpert::py::img_rgb) {
            *format = GL_RGB;
        } else if (image_->format() == dexpert::py::img_rgb) {
            *format = GL_RGBA;
        }
    }
}

bool FramePanel::saveFile() {
    if (!image_) {
        return false;
    }
    std::string path = choose_image_to_save(&current_open_dir_);
    if (!path.empty()) {
        bool success = false;
        const char *message = "Unexpected error. Callback to open image not called";
        auto cb = dexpert::py::save_image(path.c_str(), image_, [&success, &message] (bool status, const char* msg) {
            success = status;
            message = msg;
        });
        dexpert::py::get_py()->execute_callback(cb);
        if (!success) {
            show_error(message);
        }
        return success;
    }
    return false;
}

bool FramePanel::generateImage() {
    bool success = false;
    std::wstring model = dexpert::getConfig().sdModelsDir() + L"/model.safetensors";
    const char *message = "Unexpected error. Callback to generate image not called";
    std::shared_ptr<dexpert::py::RawImage> image;
    dexpert::py::txt2img_config_t cfg;
    cfg.prompt = L"An astrounaut riding a horse at the moon";
    cfg.negative = L"drawing,cartoon,3d,render,rendering";
    cfg.model = model.c_str();
    
    auto cb = dexpert::py::txt2_image(cfg, [&image, &success, &message] (bool status, const char* msg, std::shared_ptr<dexpert::py::RawImage> img) {
        success = status;
        message = msg;
        image = img;
    });

    dexpert::py::get_py()->execute_callback(cb);
    if (!success) {
        show_error(message);
    } else if (image) {
        image_ = image;
        redraw();
    }

    return success;
}

}  // namespace dexpert