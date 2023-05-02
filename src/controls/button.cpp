/*
 * Copyright (C) 2018 by Rodrigo Antonio de Araujo
 */
#include "src/controls/button.h"

namespace dexpert {

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

}  // namespace dexpert