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
    src_type_ = image_src_input;
}

FramePanel::FramePanel(image_ptr_t image, uint32_t x, uint32_t y, uint32_t w, uint32_t h) : OpenGlPanel(x, y, w, h) {
    image_ = image;
    src_type_ = image_src_self;
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

void FramePanel::addButton(int id, float xcoord, float ycoord, dexpert::xpm::xpm_t image, frame_btn_cb_t cb) {
    frame_button_t btn;
    btn.id = id;
    btn.cb = cb;
    btn.image = dexpert::xpm::image(image);
    btn.x = xcoord;
    btn.y = ycoord;
    buttons_.push_back(btn);
}

void FramePanel::get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) {
    RawImage *img = NULL;
    switch (src_type_) {
        case image_src_self:
            img = image_.get();
            break;
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

void FramePanel::get_button_coords(frame_button_t *b, float *x, float *y, int *w, int *h) {
    float &bx = *x;
    float &by = *y;
    int &bw = *w;
    int &bh = *h;
    bx = b->x;
    by = b->y;
    bx += 1.0;
    by = 1.0 - by;
    bw = b->image->w();
    bh = b->image->h();
    float sx = ((2.0 / (float) this->w()) * bw) / 2.0;
    float sy = ((2.0 / (float) this->h()) * bh) / 2.0;
    bx -= sx;
    by -= sy;
    sx *= 2;
    sy *= 2;
    if (bx < 0.0)
        bx = 0;
    if (by < 0.0)
        by = 0;
    if (bx + sx > 2.0)
        bx = 2.0 - sx;
    if (by + sy > 2.0)
        by = 2.0 - sy;
    bx -= 1.0;
    by = 1.0 - by;
}

frame_button_t *FramePanel::get_button_mouse(int x, int y) {
    float sx = (2.0 / (float) this->w());
    float sy = (2.0 / (float) this->h());
    float cx = (sx * x);
    float cy = (sy * y) ;
    float bx, by;
    int bw, bh;
    for (size_t i = 0; i < buttons_.size(); ++i) {
        auto &b = buttons_[i];
        get_button_coords(&b, &bx, &by, &bw, &bh);
        bx += 1.0;
        by = 1.0 - by;
        if (cx >= bx && cy >= by) {
            bx += (bw * sx);
            by += (bh * sy);
            if (cx <= bx && cy <= by) {
                return &b;
            }
        }
    }
    return NULL;
}

void FramePanel::mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) {
    frame_button_t * b = get_button_mouse(up_x, up_y);
    if (b) {
        b->cb(this, b->id);
    }
}


frame_button_t *FramePanel::get_button_near_mouse(int x, int y) {
    return NULL;
}


void FramePanel::draw_next() {
    float x;
    float y;
    int w;
    int h;
    int format;
    glPixelZoom(1.0, -1.0);
    
    for (size_t i = 0; i < buttons_.size(); ++i) {
        auto &b = buttons_[i];
        get_button_coords(&b, &x, &y, &w, &h);
        glRasterPos2f(x, y);

        if (w % 4 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        else
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        int format = GL_LUMINANCE;
        if (b.image->d() == 3) {
            format = GL_RGB;
        } else if (b.image->d() == 4) {
            format = GL_RGBA;
        }

        glDrawPixels(w, h, format, GL_UNSIGNED_BYTE, b.image->data()[0]);
    }
    
    glRasterPos2f(0.0f, 0.0f);
    glPixelZoom(1.0f, 1.0f);
}


}  // namespace dexpert