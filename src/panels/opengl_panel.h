/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_OPENGL_PANEL_H_
#define SRC_PANELS_OPENGL_PANEL_H_

#include <inttypes.h>
#include <FL/Fl_Gl_Window.H>

#include "src/opengl_utils/view_port.h"

#define kT

namespace dexpert
{

class OpenGlPanel: public Fl_Gl_Window {
  public:
    OpenGlPanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~OpenGlPanel();
    virtual void cancel_operations();
    const viewport_t & view_port() const;
    //void resize(int x, int y, int w, int h) override;
    void redraw();

 protected:
    virtual void get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format, int buffer_type) {};
    virtual void mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y) {};
    virtual void mouse_down(bool left_button, bool right_button, int down_x, int down_y) {};
    virtual void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) {};
    virtual void mouse_cancel() {};
    virtual void draw_next() {};
    
 protected:
    int handle(int event) override;
    void draw() override;
    void draw_overlay() override;
    void draw_buffer(const unsigned char *buffer, uint32_t w, uint32_t h, int format);

 private:
    bool mouse_down_left_;
    bool mouse_down_right_;
    int mouse_down_x_;
    int mouse_down_y_;
    viewport_t vp_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_OPENGL_PANEL_H_