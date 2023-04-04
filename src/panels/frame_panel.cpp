#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/panels/frame_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"
#include "src/stable_diffusion/state.h"

namespace dexpert {

FramePanel::FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h): OpenGlPanel(x, y, w, h) {

}

FramePanel::~FramePanel() {

}

void FramePanel::get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) {
    auto img = get_sd_state()->getInputImage();
    if (img) {
        *buffer = img->buffer();
        *w = img->w();
        *h = img->h();
        *format = GL_LUMINANCE;
        if (img->format() == dexpert::py::img_rgb) {
            *format = GL_RGB;
        } else if (img->format() == dexpert::py::img_rgb) {
            *format = GL_RGBA;
        }
    }
}

}  // namespace dexpert