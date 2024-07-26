#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "simple-ui/panel.h"


namespace dfe_ui
{
    
Panel::Panel(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    this->bg_color(100, 100, 100, 255);
}

Panel::~Panel() {
}

void Panel::paint(void *render_window) {
    if (!visible()) {
        return;
    }
    sf::RectangleShape rect(sf::Vector2f(abs_w(), abs_h()));
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(bg_color_));
    rect.setOutlineColor(sf::Color(fg_color_));
    static_cast<sf::RenderWindow *>(render_window)->draw(rect);
}

void Panel::bg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    bg_color_ = RGBA_TO_COLOR(r, g, b, a);
}

void Panel::fg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    fg_color_ = RGBA_TO_COLOR(r, g, b, a);
}

uint8_t Panel::bg_color_r() {
    return RGBA_R(bg_color_);
}

uint8_t Panel::bg_color_g() {
    return RGBA_G(bg_color_);
}

uint8_t Panel::bg_color_b() {
    return RGBA_B(bg_color_);
}

uint8_t Panel::bg_color_a() {
    return RGBA_A(bg_color_);
}

uint8_t Panel::fg_color_r() {
    return RGBA_R(fg_color_);    
}

uint8_t Panel::fg_color_g() {
    return RGBA_G(fg_color_);    
}

uint8_t Panel::fg_color_b() {
    return RGBA_B(fg_color_);    
}

uint8_t Panel::fg_color_a() {
    return RGBA_A(fg_color_);    
}

void Panel::set_on_mouse_enter(cb_event_t cb) {
    on_mouse_enter_ = cb;
}

void Panel::set_on_mouse_exit(cb_event_t cb) {
    on_mouse_exit_ = cb;
}

void Panel::mouse_enter()  {
    if (on_mouse_enter_) {
        on_mouse_enter_(this);
    }
}

void Panel::mouse_exit() {
    if (on_mouse_exit_) {
        on_mouse_exit_(this);
    }
}

} // namespace dfe_ui
