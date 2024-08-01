#pragma once

#include <inttypes.h>

namespace sf {
    class RenderTarget;
}

namespace dfe_ui
{
    
class Drawing {
    public:
        enum Direction {
            direction_top,
            direction_left,
            direction_right,
            direction_bottom
        };

        enum Type {
            drawing_flat_box,
            drawing_arrow
        };

        Drawing(Type drawing_type);
        Drawing(Type drawing_type, Direction direction);
        virtual ~Drawing();

        void color(uint32_t value);
        uint32_t color();
        void outline_color(uint32_t value);
        uint32_t outline_color();
        void size(int w, int h);
        void position(int x, int y);
        int x();
        int y();
        int h();
        int w();
        void margin(int value);
        int margin();
        void draw(sf::RenderTarget *render_target);

    private:
        void draw_arrow(sf::RenderTarget *render_target);
        void draw_box(sf::RenderTarget *render_target);

    private:
        Type m_drawing_type = drawing_flat_box;
        Direction m_direction = direction_top;
        uint32_t m_color = 0xFFFFFFFF;
        uint32_t m_outline_color = 0x000000FF;
        int m_x = 0;
        int m_y = 0;
        int m_w = 0;
        int m_h = 0;
        int m_margin = 0;
};

} // namespace dfe_ui
