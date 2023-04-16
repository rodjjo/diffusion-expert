#include <stdio.h>
#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/panels/opengl_panel.h"

namespace dexpert {

OpenGlPanel::OpenGlPanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h): Fl_Gl_Window(x, y, w, h) {
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

OpenGlPanel::~OpenGlPanel() {
}

int OpenGlPanel::handle(int event) {
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

void OpenGlPanel::resize(int x, int y, int w, int h) {
    Fl_Gl_Window::resize(x, y, w, h);
}

void OpenGlPanel::draw_buffer(const unsigned char *buffer, uint32_t w, uint32_t h, int format) {
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

void OpenGlPanel::draw()  {
    if (!valid()) {
        valid(1);
        glLoadIdentity();
        glViewport(0, 0, this->w(), this->h());
    }
    
    vp_.update(); // update the opengl view port

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    const unsigned char *buffer = NULL;
    uint32_t w = 0;
    uint32_t h = 0;
    int format = GL_RGB;

    get_buffer(&buffer, &w, &h, &format, false);
    draw_buffer(buffer, w, h, format);
    draw_next();
}

void OpenGlPanel::draw_overlay() {
    // do nothing...
}

void OpenGlPanel::cancel_operations() {
    mouse_down_left_ = false;
    mouse_down_right_ = false;
    mouse_cancel(); // cancels previous operation
    this->redraw();
}

const viewport_t & OpenGlPanel::view_port() const {
    return vp_;
}

}  // namespace dexpert
