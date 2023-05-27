/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_MINIATURE_H_
#define SRC_PANELS_MINIATURE_H_

#include <inttypes.h>
#include <FL/Fl_Gl_Window.H>

#include "src/python/raw_image.h"
#include "src/opengl_utils/view_port.h"


namespace dexpert
{

typedef enum {
    miniature_click_left,
    miniature_click_right,
    miniature_wheel_up,
    miniature_wheel_down
} miniature_event_t;

class Miniature: public Fl_Gl_Window {
  public:
    Miniature(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~Miniature();
    virtual void cancel_operations();
    const viewport_t & view_port() const;
    void setPicture(image_ptr_t image);
    RawImage *getPicture();
    void clearPicture();
    void setTag(size_t value);
    size_t getTag();

 protected:
    virtual void mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y) {};
    virtual void mouse_down(bool left_button, bool right_button, int down_x, int down_y) {};
    virtual void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y);
    virtual void mouse_cancel() {};
    virtual void draw_next() {};
   static void imageRefresh(void *cbdata);
   void imageRefresh();

 protected:
    int handle(int event) override;
    void draw() override;
    void draw_overlay() override;
    void draw_buffer(const unsigned char *buffer, uint32_t w, uint32_t h, int format);

 private:
    image_ptr_t image_;
    bool should_refresh_ = false;
    bool mouse_down_left_;
    bool mouse_down_right_;
    int mouse_down_x_;
    int mouse_down_y_;
    size_t tag_;
    viewport_t vp_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_MINIATURE_H_