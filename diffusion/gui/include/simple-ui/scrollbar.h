#pragma once

#include "simple-ui/events.h"
#include "simple-ui/component.h"

namespace dfe_ui
{

    
class Scrollbar : public Component {
    
    public:
        Scrollbar(Window * window, int x, int y, int w, int h, bool vertical);
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
        virtual void paint(sf::RenderTarget *render_target) override;

        void onchange(component_event_t value);
        component_event_t onchange();
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
        component_event_t m_onchange;
        component_cursor_t m_current_cursor = cursor_arrow;
        int         m_mouse_in_region = -1;
        bool        m_mouse_inside = false;
        bool        m_mouse_pressed = false;
        bool        m_vertical = false;
        int         m_page_size = 5;
        int         m_min = 0;
        int         m_max = 100;
        int         m_value = 0;
        int         m_mouse_down_value = 0;
        int         m_mouse_down_coord = 0;
        int         m_mouse_down_region = -1;
        int64_t     m_mouse_down_time = 0;
        uint32_t    m_arrow_size = 30;
        uint32_t    m_scroll_color = 0;
        uint32_t    m_scroll_highlighted_color = 0;
        uint32_t    m_arrow_color = 0xFFFFFFFF;
        uint32_t    m_color = 0xFFFFFFFF;
        uint32_t    m_outline_color = 0x000000FF;
        uint32_t    m_highlighted_color = 0xFFFFFFFF;
        uint32_t    m_pressed_color = 0x666666FF;
        
};

} // namespace dfe_ui
