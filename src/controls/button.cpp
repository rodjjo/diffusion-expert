/*
 * Copyright (C) 2018 by Rodrigo Antonio de Araujo
 */
#include <FL/Fl_RGB_Image.H>

#include "src/controls/button.h"

namespace dexpert {

Button::Button(callback_t callback) {
    button_ = new Fl_Button(0, 0, 1, 1, "");
    callback_ = callback;
    button_->clear_visible_focus();
    button_->callback(button_callback, this);
    data_.reset((uint8_t *)malloc(16 * 16 * 3));
    image_.reset(
        new Fl_RGB_Image(data_.get(), 16, 16, 3)
    );
    button_->image(image_.get());
    button_->align(FL_ALIGN_IMAGE_BACKDROP);
    button_->hide();
    setColor(0, 0, 0);
    button_->show();
}

Button::Button(const char *label, callback_t callback) {
    button_ = new Fl_Button(0, 0, 1, 1, label);
    callback_ = callback;
    button_->clear_visible_focus();
    button_->callback(button_callback, this);
}

Button::Button(std::shared_ptr<Fl_Image> image, callback_t callback) {
    image_ = image;
    callback_ = callback;
    button_ = new Fl_Button(0, 0, 1, 1);
    button_->clear_visible_focus();
    button_->image(image_.get());
    button_->align(FL_ALIGN_IMAGE_BACKDROP);
    button_->callback(button_callback, this);
}

bool Button::down() {
    if (!down_up_) {
        return false;
    }
    return button_->value() != 0;
}

void Button::down(bool value) {
    if (down_up_) {
        button_->value((int)value);
    }
}

void Button::enableDownUp()  {
    down_up_ = true;
}

int Button::x() {
    return button_->x();
}

int Button::y() {
    return button_->y();
}

int Button::w() {
    return button_->w();
}

void Button::size(int sw, int sh) {
    button_->size(sw, sh);
}

void Button::position(int px, int py) {
    button_->position(px, py);
}

void Button::button_callback(Fl_Widget* widget, void *userdata) {
    ((Button *) userdata)->button_callback();
}

void Button::button_callback() {
    down(!down());
    if (callback_) {
        callback_();
    }
}

int Button::h() {
    return button_->h();
}

void Button::change_label(const char *label) {
    button_->copy_label(label);
}

void Button::tooltip(const char *text) {
    button_->tooltip(text);
}

void Button::shortcut(int value) {
    button_->shortcut(value);
}

void Button::hide() {
    button_->hide();
}

void Button::show() {
    button_->show();
}

bool Button::enabled() {
    return button_->active_r();
}

void Button::enabled(bool value) {
    if (value) {
        button_->activate();
    } else {
        button_->deactivate();
    }
}

void Button::setColor(uint8_t r, uint8_t g, uint8_t b) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    
    if (image_ && image_->d() == 3 && data_.get()) {
        const int size = image_->w() * image_->h();
        uint8_t *p = data_.get();
        for (int i = 0; i < size; ++i) {
            memcpy(p, color_, sizeof(color_));
            p += sizeof(color_);
        }
        image_->uncache();
        if (button_->visible_r()) {
            button_->redraw();
        }
    }
}

void Button::getColor(uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = color_[0];
    *g = color_[1];
    *b = color_[2];
}

}  // namespace dexpert