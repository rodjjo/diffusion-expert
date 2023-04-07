#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include <CImg.h>

#include "src/panels/frame_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"
#include "src/stable_diffusion/state.h"

using namespace cimg_library;

namespace dexpert {

int abs_i(int v) {
    if (v < 0)
        return -v;
    return v;
}

FramePanel::FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h): OpenGlPanel(x, y, w, h) {

}

FramePanel::~FramePanel() {
    if (cache_buffer_) {
        free(cache_buffer_);
    }
}

void FramePanel::setGridLocation(int col, int row) {
    col_ = col;
    row_ = row;
}

void FramePanel::enableGrid() {
    grid_enabled_ = true;
}

void FramePanel::enableCache() {
    cache_enabled_ = true;
}

size_t FramePanel::gridRow() {
    return row_;
}

size_t FramePanel::gridCol() {
    return col_;
}

void FramePanel::setImageSource(image_src_t src_type) {
    src_type_ = src_type;
}

void FramePanel::update_cache(const unsigned char **buffer, uint32_t *w, uint32_t *h, int format, size_t version) {
    if (!cache_enabled_) {
        return;
    }
    
    uint32_t nw = *w;
    uint32_t nh = *h; 
    view_port().fit(&nw, &nh);

    bool force_update = false;
    // if the component resize to much (10%), force an update of the cache
    if (cache_w_ > 0 && cache_h_ > 0) {
        float dx = abs_i(nw - cache_w_) / (float)cache_w_;
        float dy = abs_i(nh - cache_h_) / (float)cache_h_;
        force_update = (dx > 0.1) || (dy > 0.1);
    } else {
        force_update = true;
    }

    if (force_update || version != cache_version_ || cache_buffer_ == NULL) {
        size_t pixel_size = 1;
        if (format == GL_RGB)
            pixel_size = 3;
        else if (format == GL_RGBA)
            pixel_size = 4;
        size_t bytes = nw * nh * pixel_size;
        
        if ((cache_buffer_ == NULL) || (cache_bytes_ != bytes)) {
            cache_bytes_ = bytes;
            cache_buffer_ = (unsigned char *)realloc(cache_buffer_, cache_bytes_);
        }
        
        cache_w_ = nw;
        cache_h_ = nh;
        cache_type_ = format;
        CImg<unsigned char> src(*buffer, *w, *h, 1, pixel_size, true);
        CImg<unsigned char> img(cache_buffer_, cache_w_, cache_h_, 1, pixel_size, true);
        
        img.draw_image(0, 0, src.get_resize(cache_w_, cache_h_));
        // TODO: draw into the buffer
    }

    *buffer = cache_buffer_;
    *w = cache_w_;
    *h = cache_h_;
    cache_version_ = version;
}

void FramePanel::get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) {
    auto img = get_sd_state()->getInputImage();
    if (!img) return;

    *buffer = img->buffer();
    *w = img->w();
    *h = img->h();
    *format = GL_LUMINANCE;
    if (img->format() == dexpert::py::img_rgb) {
        *format = GL_RGB;
    } else if (img->format() == dexpert::py::img_rgba) {
        *format = GL_RGBA;
    }

    update_cache(buffer, w, h, *format, img->getVersion());
}

}  // namespace dexpert