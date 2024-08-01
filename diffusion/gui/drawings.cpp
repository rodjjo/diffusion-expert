#include <SFML/Graphics.hpp>

#include "drawings.h"

namespace dfe_ui
{

namespace 
{
    
} // namespace 



Drawing::Drawing(Type drawing_type) : m_drawing_type(drawing_type), m_direction(direction_top) {
}

Drawing::Drawing(Type drawing_type, Direction direction) : m_drawing_type(drawing_type), m_direction(direction)  {
}

Drawing::~Drawing() {
}

void Drawing::color(uint32_t value) {
    m_color = value;
}

uint32_t Drawing::color() {
    return m_color;
}

void Drawing::outline_color(uint32_t value) {
    m_outline_color = value;   
}

uint32_t Drawing::outline_color() {
    return m_outline_color;
}

void Drawing::size(int w, int h) {
    m_w = w;
    m_h = h;
}

void Drawing::position(int x, int y) {
    m_x = x;
    m_y = y;
}

int Drawing::x() {
    return m_x;
}

int Drawing::y() {
    return m_y;
}

int Drawing::h() {
    return m_h;
}

int Drawing::w() {
    return m_w;
}

void Drawing::draw(sf::RenderTarget *render_target) {
    switch (m_drawing_type)
    {
        case drawing_arrow:
            draw_arrow(render_target);
        break;

        case drawing_flat_box:
            draw_box(render_target);
        break;

        default:
            break;
    }
}

void Drawing::draw_box(sf::RenderTarget *render_target) {
    sf::RectangleShape shape({(float)m_w, (float)m_h});
    shape.setPosition({m_x, m_y});
    shape.setFillColor(sf::Color(m_color));
    shape.setOutlineColor(sf::Color(m_outline_color));
    shape.setOutlineThickness(-1.0);
    render_target->draw(shape);
}

void Drawing::draw_arrow(sf::RenderTarget *render_target) {
    sf::Vertex line[3];
    switch (m_direction)
    {
        case direction_top: {
            line[0] = {sf::Vector2f(m_x, m_y + m_h), sf::Color(m_color)};
            line[1] = {sf::Vector2f(m_x + (m_w >> 1), m_y), sf::Color(m_color)};
            line[2] = {sf::Vector2f(m_x + m_w, m_y + m_h), sf::Color(m_color)};
        } break;
        case direction_bottom: {
            line[0] = {sf::Vector2f(m_x, m_y), sf::Color(m_color)};
            line[1] = {sf::Vector2f(m_x + (m_w >> 1), m_y  + m_h), sf::Color(m_color)};
            line[2] = {sf::Vector2f(m_x + m_w, m_y), sf::Color(m_color)};
        } break;
        case direction_left: {
            line[0] = {sf::Vector2f(m_x, m_y + (m_h >> 1)), sf::Color(m_color)};
            line[1] = {sf::Vector2f(m_x + m_w, m_y), sf::Color(m_color)};
            line[2] = {sf::Vector2f(m_x + m_w, m_y + m_h), sf::Color(m_color)};
        } break;
        case direction_right: {
            line[0] = {sf::Vector2f(m_x + m_w, m_y + (m_h >> 1)), sf::Color(m_color)};
            line[1] = {sf::Vector2f(m_x, m_y), sf::Color(m_color)};
            line[2] = {sf::Vector2f(m_x, m_y + m_h), sf::Color(m_color)};
        } break;
        default:
            break;
    }
    render_target->draw(line, 3, sf::PrimitiveType::Triangles);
    line[0].color = sf::Color(m_outline_color);
    line[1].color = sf::Color(m_outline_color);
    line[2].color = sf::Color(m_outline_color);
    render_target->draw(line, 3, sf::PrimitiveType::LineStrip);
}

void Drawing::margin(int value) {
    m_margin = value;
}

int Drawing::margin() {
    return m_margin;
}
    
} // namespace dfe_ui
