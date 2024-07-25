#pragma once

#include <string>
#include "simple-ui/icons.h"
#include "simple-ui/component.h"

namespace dfe_ui
{
    class Button : public Component {
        public:
            enum icon_position_t {
                icon_center,
                icon_left,
                icon_right,
                icon_left_most,
                icon_right_most
            };

            Button(int x, int y, int w, int h, const std::wstring &text, icon_type_t icon_type=img_none);
            virtual ~Button();
            std::wstring text();
            void text(const std::wstring &value);
            int character_size();
            void character_size(int value);
            void text_color(uint32_t color);
            uint32_t text_color();
            virtual void paint(void *render_window) override;
            void color(uint32_t value);
            uint32_t color();
            void outline_color(uint32_t value);
            uint32_t outline_color();
            icon_position_t icon_position();
            void icon_position(icon_position_t value);
            void highlighted_color(uint32_t color);
            uint32_t highlighted_color();
            uint32_t pressed_color();
            void pressed_color(uint32_t value);

            void checked_icon(icon_type_t icon_type);
            icon_type_t checked_icon();
            bool checked();
            void checked(bool value);

        protected:
            virtual void mouse_enter() override;
            virtual void mouse_exit() override;
            virtual void handle_mouse_left_pressed(int x, int y) override;
            virtual void handle_mouse_left_released(int x, int y) override;
            bool clickable() override;
            component_cursor_t cursor() override;

        private:
            void update_text_min_y();

        private:
            icon_type_t             checked_icon_ = img_none;
            bool                    checked_ = false;
            bool                    mouse_pressed_ = false;
            bool                    mouse_inside_ = false;
            icon_position_t         icon_pos_ = icon_center;
            icon_type_t             icon_type_ = img_none;
            int                     text_min_y_ = 0;
            std::shared_ptr<void>   text_;
            int                     character_size_ = 30;
            uint32_t                text_color_ = RGBA_TO_COLOR(255, 255, 255, 255);
            uint32_t                color_ = 0xFFFFFFFF;
            uint32_t                outline_color_ = 0x000000FF;
            uint32_t                highlighted_color_ = 0xFFFFFFFF;
            uint32_t                pressed_color_ = 0x666666FF;
    };
} // namespace dfe_ui
