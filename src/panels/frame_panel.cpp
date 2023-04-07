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

void FramePanel::setGridLocation(int index, int variation) {
    index_ = index;
    variation_ = variation;
}

void FramePanel::enableGrid() {
    grid_enabled_ = true;
}

void FramePanel::enableCache() {
    cache_enabled_ = true;
}

size_t FramePanel::gridIndex() {
    return index_;
}

size_t FramePanel::gridVariation() {
    return variation_;
}

void FramePanel::setImageSource(image_src_t src_type) {
    src_type_ = src_type;
}

void FramePanel::update_cache(const unsigned char **buffer, uint32_t *w, uint32_t *h, int channels, size_t version) {
    if (!cache_enabled_) {
        return;
    }
    
    uint32_t nw = *w;
    uint32_t nh = *h; 

    float greater_x = (this->w() > 0) ? nw / (float)this->w() : 1.0;
    float greater_y = (this->h() > 0) ? nh / (float)this->h() : 1.0;

    if (greater_x > greater_y) {
        greater_y = greater_x;
    }

    if (greater_y > 1.2) { // the image is 20% bigger
        greater_y = 1.0 / greater_y;
        nw *= greater_y;
        nh *= greater_y;
    } else {
        return; // do not use cache, just draw the original image
    }

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
        size_t bytes = nw * nh * channels;
        
        if ((cache_buffer_ == NULL) || (cache_bytes_ != bytes)) {
            cache_bytes_ = bytes;
            cache_buffer_ = (unsigned char *)realloc(cache_buffer_, cache_bytes_);
        }
       
        cache_w_ = nw;
        cache_h_ = nh;
        cache_channels_ = channels;
        
        CImg<unsigned char> src(*buffer, channels, *w, *h, 1, true);
        CImg<unsigned char> img(cache_buffer_, channels, cache_w_, cache_h_, 1, true);
        src.permute_axes("yzcx");
        img.permute_axes("yzcx");
        img.draw_image(0, 0, src.get_resize(cache_w_, cache_h_));
        img.permute_axes("cxyz");
        src.permute_axes("cxyz");

        cache_version_ = version;
    }

    *buffer = cache_buffer_;
    *w = cache_w_;
    *h = cache_h_;
    
}

void FramePanel::get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) {
    RawImage *img = NULL;
    switch (src_type_) {
        case image_src_input:
            //img = get_sd_state()->getInputImage();
            break;
        case image_src_results:
            img = get_sd_state()->getResultsImage(index_, variation_);
            break;
        case image_src_input_mask:
            //img = get_sd_state()->getInputMaskImage();
            break;
        case image_src_input_scribble:
            // img = get_sd_state()->getInputScribbleImage();
            break;
        case image_src_input_pose:
            // img = get_sd_state()->getInputPoseImage();
            break;
        case image_src_controlnet1:
            // img = get_sd_state()->getControlNetImage(0);
            break;
        case image_src_controlnet2:
            // img = get_sd_state()->getControlNetImage(1);
            break;
        case image_src_controlnet3:
            // img = get_sd_state()->getControlNetImage(2);
            break;
        case image_src_controlnet4:
            // img = get_sd_state()->getControlNetImage(3);
            break;
        default:
            img = NULL; 
        break;
    }

    if (!img) return;

    *buffer = img->buffer();
    *w = img->w();
    *h = img->h();
    *format = GL_LUMINANCE;
    int channels = 1;
    if (img->format() == dexpert::py::img_rgb) {
        *format = GL_RGB;
        channels = 3;
    } else if (img->format() == dexpert::py::img_rgba) {
        *format = GL_RGBA;
        channels = 4;
    }

    update_cache(buffer, w, h, channels, img->getVersion());
}

}  // namespace dexpert