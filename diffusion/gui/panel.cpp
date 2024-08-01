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

void Panel::paint(sf::RenderTarget *render_target) {
    if (!visible()) {
        return;
    }
    sf::RectangleShape rect(sf::Vector2f(abs_w(), abs_h()));
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(m_bg_color));
    rect.setOutlineColor(sf::Color(m_fg_color));
    render_target->draw(rect);
}

void Panel::bg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    m_bg_color = RGBA_TO_COLOR(r, g, b, a);
}

void Panel::fg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    m_fg_color = RGBA_TO_COLOR(r, g, b, a);
}

uint8_t Panel::bg_color_r() {
    return RGBA_R(m_bg_color);
}

uint8_t Panel::bg_color_g() {
    return RGBA_G(m_bg_color);
}

uint8_t Panel::bg_color_b() {
    return RGBA_B(m_bg_color);
}

uint8_t Panel::bg_color_a() {
    return RGBA_A(m_bg_color);
}

uint8_t Panel::fg_color_r() {
    return RGBA_R(m_fg_color);    
}

uint8_t Panel::fg_color_g() {
    return RGBA_G(m_fg_color);    
}

uint8_t Panel::fg_color_b() {
    return RGBA_B(m_fg_color);    
}

uint8_t Panel::fg_color_a() {
    return RGBA_A(m_fg_color);    
}

void Panel::set_on_mouse_enter(cb_event_t cb) {
    m_on_mouse_enter = cb;
}

void Panel::set_on_mouse_exit(cb_event_t cb) {
    m_on_mouse_exit = cb;
}

void Panel::mouse_enter()  {
    if (m_on_mouse_enter) {
        m_on_mouse_enter(this);
    }
}

void Panel::mouse_exit() {
    if (m_on_mouse_exit) {
        m_on_mouse_exit(this);
    }
}

} // namespace dfe_ui
