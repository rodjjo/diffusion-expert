#pragma once

#include <functional>
#include <string>
#include "simple-ui/icons.h"
#include "simple-ui/component.h"

namespace sf {
    class Text;
}

namespace dfe_ui
{
    typedef std::function<void(Component* self)> cb_button_click_t;

    class Button : public Component {
        public:
            enum icon_position_t {
                icon_center,
                icon_left,
                icon_right,
                icon_left_most,
                icon_right_most
            };

            Button(Window * window, int x, int y, int w, int h, const std::wstring &text, icon_type_t icon_type=img_none);
            virtual ~Button();
            std::wstring text();
            void text(const std::wstring &value);
            int character_size();
            void character_size(int value);
            void text_color(uint32_t color);
            uint32_t text_color();
            virtual void paint(sf::RenderTarget *render_target) override;
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

            void onclick(cb_button_click_t value);

        protected:
            virtual void mouse_enter() override;
            virtual void mouse_exit() override;
            virtual void handle_mouse_left_pressed(int x, int y) override;
            virtual void handle_mouse_left_released(int x, int y) override;
            virtual void handle_click() override;
            
            bool clickable() override;
            component_cursor_t cursor() override;

        private:
            void update_text_min_y();

        private:
            icon_type_t             m_checked_icon = img_none;
            bool                    m_checked = false;
            bool                    m_mouse_pressed = false;
            bool                    m_mouse_inside = false;
            icon_position_t         m_icon_pos = icon_center;
            icon_type_t             m_icon_type = img_none;
            int                     m_text_min_y = 0;
            std::shared_ptr<sf::Text>   m_text;
            int                     m_character_size = 30;
            uint32_t                m_text_color = RGBA_TO_COLOR(255, 255, 255, 255);
            uint32_t                m_color = 0xFFFFFFFF;
            uint32_t                m_outline_color = 0x000000FF;
            uint32_t                m_highlighted_color = 0xFFFFFFFF;
            uint32_t                m_pressed_color = 0x666666FF;
            cb_button_click_t       m_on_click;
    };
} // namespace dfe_ui
