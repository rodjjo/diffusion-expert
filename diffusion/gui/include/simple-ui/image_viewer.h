#pragma once

#include "simple-ui/component.h"
#include "simple-ui/raw_image.h"


namespace sf {

class Texture;
class Sprite;

} // namespace sf


namespace dfe_ui
{
    class ImageViewer : public Component {
        public:
            ImageViewer(Window *window, int x, int y, int w, int h);
            virtual ~ImageViewer();
            image_ptr_t image();
            void image(image_ptr_t value);
            virtual void paint(sf::RenderTarget *render_target) override;
            bool fit_image();
            void fit_image(bool value);
            void image_scale(float scale);
            float image_scale();
            void image_scroll_x(int value);
            void image_scroll_y(int value);
            int image_scroll_x();
            int image_scroll_y();

        protected:
            void handle_mouse_wheel(int8_t direction, int x, int y) override;
            void handle_mouse_left_pressed(int x, int y) override;
            void handle_mouse_left_released(int x, int y) override;
            void handle_mouse_moved(int x, int y) override;
            component_cursor_t cursor() override;
            bool clickable() override;
        private:
            void update_texture();
            std::pair<int, int> get_image_origin();
            float compute_scale();

        private:
            bool m_mouse_control = false;
            bool m_mouse_pressed = false;
            uint32_t m_mouse_down_x = 0;
            uint32_t m_mouse_down_y = 0;
            bool m_fit_image = true;
            int m_image_scroll_x = 0;
            int m_image_scroll_y = 0;
            float m_image_scale = 1.0;
            int64_t m_version = 0;
            image_ptr_t m_image;
            std::shared_ptr<sf::Texture> m_texture;
            std::shared_ptr<sf::Sprite> m_sprite;
    };
} // namespace dfe_ui
