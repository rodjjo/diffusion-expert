#include "simple-ui/arrow_button.h"
#include "simple-ui/theme.h"
#include "simple-ui/win.h"
#include "drawings.h"

namespace dfe_ui {
        
ArrowButton::ArrowButton(Window * window, int x, int y, int w, int h, arrow_direction_t direction) : Component(window), m_direction(direction) {
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
    m_mouse_pressed = true;
}

void ArrowButton::handle_mouse_left_released(int x, int y) {
    m_mouse_pressed = false;
}

void ArrowButton::arrow_color(uint32_t color) {
    m_arrow_color = color;
}

uint32_t ArrowButton::arrow_color() {
    return m_arrow_color;
}

void ArrowButton::mouse_enter() {
    m_mouse_inside = true;
}

void ArrowButton::mouse_exit() {
    m_mouse_inside = false;
}

bool ArrowButton::clickable() {
    return true;
}

component_cursor_t ArrowButton::cursor() {
    return cursor_hand;
}

 void ArrowButton::handle_click() {
    if (m_onclick) {
        m_onclick(this);
    }
 }

void ArrowButton::paint(sf::RenderTarget *render_target) {
    Drawing::Direction dir = Drawing::direction_top;
    if (m_direction == arrow_down) {
        dir = Drawing::direction_bottom;
    } else if (m_direction == arrom_right) {
        dir = Drawing::direction_right;
    } else if (m_direction == arrow_left) {
        dir = Drawing::direction_left;
    }

    int w = abs_w();
    int h = abs_h();
    int size = m_arrow_size * abs_scale();

    {  // box
        Drawing dw(Drawing::drawing_flat_box);
        dw.outline_color(m_outline_color);

        if (m_mouse_pressed) {
            dw.color(m_arrow_color);
        } else if (m_mouse_inside) {
            dw.color(m_highlighted_color);
        } else {
            dw.color(m_color);  
        }
        
        dw.size(abs_w(), abs_h());
        dw.position(abs_x(), abs_y());
        dw.margin(0);
        dw.draw(render_target);
    }

    {  // arrow
        Drawing dw(Drawing::drawing_arrow, dir);
        dw.outline_color(m_mouse_pressed ? m_pressed_color : m_arrow_color );
        dw.color(m_mouse_pressed ? m_pressed_color : m_arrow_color);
        dw.size(size, size);
        dw.position(abs_x() + w / 2 - size / 2, abs_y() + h / 2 - size / 2);
        dw.margin(0);
        dw.draw(render_target);
    }

}

void ArrowButton::color(uint32_t value) {
    m_color = value;
}

uint32_t ArrowButton::color() {
    return m_color;
}

void ArrowButton::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t ArrowButton::outline_color() {
    return m_outline_color;
}

void ArrowButton::highlighted_color(uint32_t color) {
    m_highlighted_color = color;
}

uint32_t ArrowButton::highlighted_color() {
    return m_highlighted_color;
}

uint32_t ArrowButton::pressed_color() {
    return m_pressed_color;
}

void ArrowButton::pressed_color(uint32_t value) {
    m_pressed_color = value;
}

int ArrowButton::arrow_size() {
    return m_arrow_size;
}

void ArrowButton::arrow_size(int value) {
    m_arrow_size = value;
}

void  ArrowButton::onclick(component_event_t value) {
    m_onclick = value;
}

component_event_t  ArrowButton::onclick() {
    return m_onclick;
}



} // namespace dfe_ui
