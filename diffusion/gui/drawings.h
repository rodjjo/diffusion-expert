#pragma once

#include <inttypes.h>

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
        void draw(void *render_window);

    private:
        void draw_arrow(void *render_window);
        void draw_box(void *render_window);

    private:
        Type drawing_type_ = drawing_flat_box;
        Direction direction_ = direction_top;
        uint32_t color_ = 0xFFFFFFFF;
        uint32_t outline_color_ = 0x000000FF;
        int x_ = 0;
        int y_ = 0;
        int w_ = 0;
        int h_ = 0;
        int margin_ = 0;
};

} // namespace dfe_ui
