#include <stdio.h>
#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/data/event_manager.h"
#include "src/panels/miniature.h"

namespace dexpert {

Miniature::Miniature(uint32_t x, uint32_t y, uint32_t w, uint32_t h): Fl_Gl_Window(x, y, w, h) {
    mouse_down_left_ = false;
    mouse_down_right_ = false;
    mouse_down_x_ = false;
    mouse_down_y_ = false;
    vp_[0] = 0;
    vp_[1] = 0;
    vp_[2] = this->w();
    vp_[3] = this->h();
    valid(0);
}

Miniature::~Miniature() {
}

void Miniature::setPicture(image_ptr_t image) {
    if (!image.get()) {
        clearPicture();
        return;
    }
    image_ = image;
    if (visible_r()) {
        redraw();
    }
}

void Miniature::clearPicture() {
    image_.reset();
    if (visible_r()) {
        redraw();
    }
}

RawImage *Miniature::getPicture() {
    return image_.get();
}

void Miniature::mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) {
    if (left_button) {
        trigger_event(this, miniature_click_left);
    } else if (right_button) {
        trigger_event(this, miniature_click_right);
    }
}

int Miniature::handle(int event) {
    switch (event) {
        case FL_KEYUP:
        case FL_KEYDOWN: {
            return Fl_Gl_Window::handle(event);
        }
        break;

        case FL_MOVE:
        case FL_DRAG: {
            if (!mouse_down_left_ && !mouse_down_right_) {
                mouse_down_x_ = Fl::event_x();
                mouse_down_y_ = Fl::event_y();
            }
            mouse_move(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_, Fl::event_x(), Fl::event_y());
        } break;

        case FL_PUSH: {
            bool mouse_down_left = Fl::event_button() == FL_LEFT_MOUSE;
            bool mouse_down_right = !mouse_down_left && Fl::event_button() == FL_RIGHT_MOUSE;

            if (mouse_down_left || mouse_down_right) {
                mouse_down_x_ = Fl::event_x();
                mouse_down_y_ = Fl::event_y();
                mouse_down_left_ = mouse_down_left;
                mouse_down_right_ = mouse_down_right;
                mouse_down(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_);
            } else {
                return Fl_Gl_Window::handle(event);
            }
        } break;

        case FL_RELEASE: {
            if (Fl::event_button() != FL_LEFT_MOUSE && Fl::event_button() != FL_RIGHT_MOUSE) {
                return Fl_Gl_Window::handle(event);
            }
            if (mouse_down_left_ || mouse_down_right_) {
                mouse_up(mouse_down_left_, mouse_down_right_,  mouse_down_x_, mouse_down_y_, Fl::event_x(), Fl::event_y());
                mouse_down_left_ = false;
                mouse_down_right_ = false;
            }
        } break;

        default:
            return Fl_Gl_Window::handle(event);
    }

    return 1;
}


void Miniature::draw_buffer(const unsigned char *buffer, uint32_t w, uint32_t h, int format) {
    if (buffer == NULL || w == 0 || h == 0) {
        return;
    }

    float pixel_zoom = vp_.raster_zoom(w, h);
    point_t raster = vp_.raster_coords(w, h);

    float px = -1.0 + raster.x;
    float py = 1.0 - raster.y;

    if (px < -1.0)
        px = -1.0;
    if (py < -1.0)
        py = -1.0;
    if (py > 1.0)
        py = 1.0;
    if (px > 1.0)
        px = 1.0;

    glRasterPos2f(px, py);
    glPixelZoom(pixel_zoom, -pixel_zoom);

    if (w % 4 == 0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glDrawPixels(w, h, format, GL_UNSIGNED_BYTE, buffer);

    glRasterPos2f(0.0f, 0.0f);
    glPixelZoom(1.0f, 1.0f);
}

void Miniature::draw()  {
    if (!valid()) {
        valid(1);
        glLoadIdentity();
        glViewport(0, 0, this->w(), this->h());
    }
    
    vp_.update(); // update the opengl view port

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    const unsigned char *buffer = NULL;
    uint32_t w = 0;
    uint32_t h = 0;
    int format = GL_RGB;

    if (!image_.get()) {
        return;
    }

    buffer = image_->buffer();
    
    w = image_->w();
    h = image_->h();
    format = GL_LUMINANCE;
    int channels = 1;
    if (image_->format() == dexpert::py::img_rgb) {
        format = GL_RGB;
        channels = 3;
    } else if (image_->format() == dexpert::py::img_rgba) {
        format = GL_RGBA;
        channels = 4;
    }
    draw_buffer(buffer, w, h, format);
    draw_next();
}

void Miniature::draw_overlay() {
    // do nothing...
}

void Miniature::cancel_operations() {
    mouse_down_left_ = false;
    mouse_down_right_ = false;
    mouse_cancel(); // cancels previous operation
    this->redraw();
}

const viewport_t & Miniature::view_port() const {
    return vp_;
}

void Miniature::setTag(size_t value) {
    tag_ = value;
}

size_t Miniature::getTag() {
    return tag_;
}


}  // namespace dexpert
