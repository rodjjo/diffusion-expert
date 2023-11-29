/*
 * Copyright (C) 2018 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_CONTROLS_BUTTON_H_
#define SRC_CONTROLS_BUTTON_H_

#include <memory>
#include <functional>

#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>


namespace dfe {

typedef std::function<void()> callback_t;

class Button {
 public:
    Button(callback_t callback);
    Button(const char *label, callback_t callback);
    Button(std::shared_ptr<Fl_Image> image, callback_t callback);
    void enableDownUp();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void getColor(uint8_t *r, uint8_t *g, uint8_t *b);
    void change_label(const char *label);
    void position(int px, int py);
    void shortcut(int value);
    void size(int sw, int sh);
    void tooltip(const char *text);
    void hide();
    void show();
    void set_visible();
    int x();
    int y();
    int w();
    int h();
    bool down();
    void down(bool value);
    bool enabled();
    void enabled(bool value);
    
 private:
    static void button_callback(Fl_Widget* widget, void *userdata);
    void button_callback();

 private:
   uint8_t color_[3] = {0,};
   bool down_up_ = false;
   callback_t callback_;

   Fl_Button *button_;
   std::shared_ptr<uint8_t> data_;
   std::shared_ptr<Fl_Image> image_;
};

}  // namespace dexpert

#endif  // SRC_CONTROLS_BUTTON_H_