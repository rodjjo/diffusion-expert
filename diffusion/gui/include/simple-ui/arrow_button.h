#pragma once

#include <string>
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
        
        ArrowButton(int x, int y, int w, int h, arrow_direction_t direction);
        virtual ~ArrowButton();
        void arrow_color(uint32_t color);
        int arrow_size();
        void arrow_size(int value);
        uint32_t arrow_color();
        virtual void paint(void *render_window) override;
        void color(uint32_t value);
        uint32_t color();
        void outline_color(uint32_t value);
        uint32_t outline_color();
        void highlighted_color(uint32_t color);
        uint32_t highlighted_color();
        uint32_t pressed_color();
        void pressed_color(uint32_t value);

    protected:
        virtual void mouse_enter() override;
        virtual void mouse_exit() override;
        virtual void handle_mouse_left_pressed(int x, int y) override;
        virtual void handle_mouse_left_released(int x, int y) override;
        bool clickable() override;
        component_cursor_t cursor() override;

    private:
        bool                mouse_inside_ = false;
        bool                mouse_pressed_ = false;
        arrow_direction_t   direction_ = arrow_up;
        uint32_t            arrow_size_ = 30;
        uint32_t            arrow_color_ = RGBA_TO_COLOR(255, 255, 255, 255);
        uint32_t            color_ = 0xFFFFFFFF;
        uint32_t            outline_color_ = 0x000000FF;
        uint32_t            highlighted_color_ = 0xFFFFFFFF;
        uint32_t            pressed_color_ = 0x666666FF;
};

};