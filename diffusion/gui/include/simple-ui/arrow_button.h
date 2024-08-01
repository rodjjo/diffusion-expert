#pragma once

#include <string>

#include "simple-ui/events.h"
#include "simple-ui/component.h"

namespace dfe_ui
{

class ArrowButton : public Component {
    public:
        enum arrow_direction_t {
            arrow_up,
            arrow_down,
            arrom_right,
            arrow_left
        };
        
        ArrowButton(Window * window, int x, int y, int w, int h, arrow_direction_t direction);
        virtual ~ArrowButton();
        int arrow_size();
        void arrow_size(int value);
        virtual void paint(sf::RenderTarget *render_target) override;
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
        void onclick(component_event_t value);
        component_event_t onclick();

    protected:
        virtual void mouse_enter() override;
        virtual void mouse_exit() override;
        virtual void handle_click() override;
        virtual void handle_mouse_left_pressed(int x, int y) override;
        virtual void handle_mouse_left_released(int x, int y) override;
        bool clickable() override;
        component_cursor_t cursor() override;

    private:
        component_event_t   m_onclick;
        bool                m_mouse_inside = false;
        bool                m_mouse_pressed = false;
        arrow_direction_t   m_direction = arrow_up;
        uint32_t            m_arrow_size = 30;
        uint32_t            m_arrow_color = 0xFFFFFFFF;
        uint32_t            m_color = 0xFFFFFFFF;
        uint32_t            m_outline_color = 0x000000FF;
        uint32_t            m_highlighted_color = 0xFFFFFFFF;
        uint32_t            m_pressed_color = 0x666666FF;
};

};