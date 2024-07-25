#include <SFML/Graphics.hpp>

#include "drawings.h"

namespace dfe_ui
{

namespace 
{
    
} // namespace 



Drawing::Drawing(Type drawing_type) : drawing_type_(drawing_type), direction_(direction_top) {
}

Drawing::Drawing(Type drawing_type, Direction direction) : drawing_type_(drawing_type), direction_(direction)  {
}

Drawing::~Drawing() {
}

void Drawing::color(uint32_t value) {
    color_ = value;
}

uint32_t Drawing::color() {
    return color_;
}

void Drawing::outline_color(uint32_t value) {
    outline_color_ = value;   
}

uint32_t Drawing::outline_color() {
    return outline_color_;
}

void Drawing::size(int w, int h) {
    w_ = w;
    h_ = h;
}

void Drawing::position(int x, int y) {
    x_ = x;
    y_ = y;
}

int Drawing::x() {
    return x_;
}

int Drawing::y() {
    return y_;
}

int Drawing::h() {
    return h_;
}

int Drawing::w() {
    return w_;
}

void Drawing::draw(void *render_window) {
    switch (drawing_type_)
    {
        case drawing_arrow:
            draw_arrow(render_window);
        break;

        case drawing_flat_box:
            draw_box(render_window);
        break;

        default:
            break;
    }
}

void Drawing::draw_box(void *render_window) {
    auto wnd = static_cast<sf::RenderWindow *>(render_window);
    sf::RectangleShape shape({(float)w_, (float)h_});
    shape.setPosition({x_, y_});
    shape.setFillColor(sf::Color(color_));
    shape.setOutlineColor(sf::Color(outline_color_));
    shape.setOutlineThickness(-1.0);
    wnd->draw(shape);
}

void Drawing::draw_arrow(void *render_window) {
    auto wnd = static_cast<sf::RenderWindow *>(render_window);
    sf::Vertex line[3];
    switch (direction_)
    {
        case direction_top: {
            line[0] = {sf::Vector2f(x_, y_ + h_), sf::Color(color_)};
            line[1] = {sf::Vector2f(x_ + (w_ >> 1), y_), sf::Color(color_)};
            line[2] = {sf::Vector2f(x_ + w_, y_ + h_), sf::Color(color_)};
        } break;
        case direction_bottom: {
            line[0] = {sf::Vector2f(x_, y_), sf::Color(color_)};
            line[1] = {sf::Vector2f(x_ + (w_ >> 1), y_  + h_), sf::Color(color_)};
            line[2] = {sf::Vector2f(x_ + w_, y_), sf::Color(color_)};
        } break;
        case direction_left: {
            line[0] = {sf::Vector2f(x_, y_ + (h_ >> 1)), sf::Color(color_)};
            line[1] = {sf::Vector2f(x_ + w_, y_), sf::Color(color_)};
            line[2] = {sf::Vector2f(x_ + w_, y_ + h_), sf::Color(color_)};
        } break;
        case direction_right: {
            line[0] = {sf::Vector2f(x_ + w_, y_ + (h_ >> 1)), sf::Color(color_)};
            line[1] = {sf::Vector2f(x_, y_), sf::Color(color_)};
            line[2] = {sf::Vector2f(x_, y_ + h_), sf::Color(color_)};
        } break;
        default:
            break;
    }
    wnd->draw(line, 3, sf::PrimitiveType::Triangles);
    line[0].color = sf::Color(outline_color_);
    line[1].color = sf::Color(outline_color_);
    line[2].color = sf::Color(outline_color_);
    wnd->draw(line, 3, sf::PrimitiveType::LineStrip);
}

void Drawing::margin(int value) {
    margin_ = value;
}

int Drawing::margin() {
    return margin_;
}
    
} // namespace dfe_ui
