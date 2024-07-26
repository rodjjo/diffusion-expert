#pragma once

#include "component.h"

namespace dfe_ui
{

    
class Scrollbar : public Component {
    
    public:
        Scrollbar(int x, int y, int w, int h, bool vertical);
        virtual ~Scrollbar();

        int min();
        void min(int value);
        int max();
        void max(int value);
        bool vertical();
        void vertical(bool value);
        int value();
        void value(int val);
        int page_size();
        void page_size(int value);

        void arrow_color(uint32_t color);
        uint32_t arrow_color();
        void color(uint32_t value);
        uint32_t color();
        void outline_color(uint32_t value);
        uint32_t outline_color();
        void highlighted_color(uint32_t color);
        uint32_t highlighted_color();
        uint32_t pressed_color();
        void pressed_color(uint32_t value);
        void scroll_color(uint32_t value);
        uint32_t scroll_color();
        void scroll_highlighted_color(uint32_t value);
        uint32_t scroll_highlighted_color();
        virtual void paint(void *render_window) override;

    protected:
        virtual void mouse_enter() override;
        virtual void mouse_exit() override;
        virtual void handle_mouse_left_pressed(int x, int y) override;
        virtual void handle_mouse_left_released(int x, int y) override;
        virtual void handle_mouse_moved(int x, int y) override;
        virtual void handle_mouse_wheel(int8_t direction, int x, int y) override;

        bool clickable() override;
        component_cursor_t cursor() override; 
        void click_scroll();

    private:
        typedef struct { int x; int y; int w; int h; } int_rect_t;
        typedef int_rect_t scrool_regions_t[5];
        void compute_regions(scrool_regions_t &regions);
        void adjust_scroll();
        void compute_mouse_region(int x, int y);
    private:
        component_cursor_t current_cursor_ = cursor_arrow;
        int         mouse_in_region_ = -1;
        bool        mouse_inside_ = false;
        bool        mouse_pressed_ = false;
        bool        vertical_ = false;
        int         page_size_ = 5;
        int         min_ = 0;
        int         max_ = 100;
        int         value_ = 0;
        int         mouse_down_value_ = 0;
        int         mouse_down_coord_ = 0;
        int         mouse_down_region_ = -1;
        int64_t     mouse_down_time_ = 0;
        uint32_t    arrow_size_ = 30;
        uint32_t    scroll_color_ = 0;
        uint32_t    scroll_highlighted_color_ = 0;
        uint32_t    arrow_color_ = 0xFFFFFFFF;
        uint32_t    color_ = 0xFFFFFFFF;
        uint32_t    outline_color_ = 0x000000FF;
        uint32_t    highlighted_color_ = 0xFFFFFFFF;
        uint32_t    pressed_color_ = 0x666666FF;
        
};

} // namespace dfe_ui
