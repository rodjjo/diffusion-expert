#include "simple-ui/arrow_button.h"
#include "simple-ui/theme.h"
#include "simple-ui/win.h"
#include "drawings.h"

namespace dfe_ui {
        
ArrowButton::ArrowButton(Window * window, int x, int y, int w, int h, arrow_direction_t direction) : Component(window), direction_(direction) {
    this->coordinates(x, y, w, h);
    arrow_color(dfe_ui::theme::button_text_color());
    color(dfe_ui::theme::button_fill_color());
    outline_color(dfe_ui::theme::button_outline_color());
    highlighted_color(dfe_ui::theme::button_highlighted_color());
    pressed_color(dfe_ui::theme::button_pressed_color());
}

ArrowButton::~ArrowButton() {
}

void ArrowButton::handle_mouse_left_pressed(int x, int y) {
    mouse_pressed_ = true;
}

void ArrowButton::handle_mouse_left_released(int x, int y) {
    mouse_pressed_ = false;
}

void ArrowButton::arrow_color(uint32_t color) {
    arrow_color_ = color;
}

uint32_t ArrowButton::arrow_color() {
    return arrow_color_;
}

void ArrowButton::mouse_enter() {
    mouse_inside_ = true;
}

void ArrowButton::mouse_exit() {
    mouse_inside_ = false;
}

bool ArrowButton::clickable() {
    return true;
}

component_cursor_t ArrowButton::cursor() {
    return cursor_hand;
}

void ArrowButton::paint(void *render_window) {
    Drawing::Direction dir = Drawing::direction_top;
    if (direction_ == arrow_down) {
        dir = Drawing::direction_bottom;
    } else if (direction_ == arrom_right) {
        dir = Drawing::direction_right;
    } else if (direction_ == arrow_left) {
        dir = Drawing::direction_left;
    }

    int w = abs_w();
    int h = abs_h();
    int size = arrow_size_ * abs_scale();

    {  // box
        Drawing dw(Drawing::drawing_flat_box);
        dw.outline_color(outline_color_);

        if (mouse_pressed_) {
            dw.color(arrow_color_);
        } else if (mouse_inside_) {
            dw.color(highlighted_color_);
        } else {
            dw.color(color_);  
        }
        
        dw.size(abs_w(), abs_h());
        dw.position(abs_x(), abs_y());
        dw.margin(0);
        dw.draw(render_window);
    }

    {  // arrow
        Drawing dw(Drawing::drawing_arrow, dir);
        dw.outline_color(mouse_pressed_ ? pressed_color_ : arrow_color_ );
        dw.color(mouse_pressed_ ? pressed_color_ : arrow_color_);
        dw.size(size, size);
        dw.position(abs_x() + w / 2 - size / 2, abs_y() + h / 2 - size / 2);
        dw.margin(0);
        dw.draw(render_window);
    }

}

void ArrowButton::color(uint32_t value) {
    color_ = value;
}

uint32_t ArrowButton::color() {
    return color_;
}

void ArrowButton::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t ArrowButton::outline_color() {
    return outline_color_;
}

void ArrowButton::highlighted_color(uint32_t color) {
    highlighted_color_ = color;
}

uint32_t ArrowButton::highlighted_color() {
    return highlighted_color_;
}

uint32_t ArrowButton::pressed_color() {
    return pressed_color_;
}

void ArrowButton::pressed_color(uint32_t value) {
    pressed_color_ = value;
}

int ArrowButton::arrow_size() {
    return arrow_size_;
}

void ArrowButton::arrow_size(int value) {
    arrow_size_ = value;
}


} // namespace dfe_ui
