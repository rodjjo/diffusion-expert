#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "simple-ui/image_viewer.h"


namespace dfe_ui
{

ImageViewer::ImageViewer(Window *window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
}

ImageViewer::~ImageViewer() {
}

image_ptr_t ImageViewer::image() {
    return m_image;
}

bool ImageViewer::fit_image() {
    return m_fit_image;
}

void ImageViewer::fit_image(bool value) {
    m_fit_image = value;
}

void ImageViewer::update_texture() {
    if (!m_image) return;
    if (m_image->version() == m_version) return;
    if (m_texture && m_texture->getSize() != sf::Vector2u{m_image->w(),  m_image->h()}) {
        m_texture.reset();
    }
    if (!m_texture) {
        auto tx = sf::Texture::create({m_image->w(),  m_image->h()}, !m_image->is_rgba());
        if (tx) {
            m_texture.reset(new sf::Texture(std::move(*tx)));
        } else {
            return;
        }
    }
    if (m_version != m_image->version()) {
        m_texture->update(m_image->buffer());
        m_version = m_image->version();
    }
}

void ImageViewer::image(image_ptr_t value) {
    m_image = value;
    m_version = m_image->version() + 1;
    update_texture();
}

void ImageViewer::image_scale(float scale) {
    m_image_scale = scale;
}

float ImageViewer::image_scale() {
    return m_image_scale;
}

std::pair<int, int> ImageViewer::get_image_origin() {
    if (!m_image) {
        return std::make_pair(0, 0);
    }

    if (m_fit_image) {
        auto scale = compute_scale();
        int abs_w = this->abs_w();
        int abs_h = this->abs_h();
        int w = m_image->w() * scale;
        int h = m_image->h() * scale;
        int sx = 0, sy = 0;

        if (w < abs_w) {
            sx = (abs_w - w) / 2;
        }

        if (h < abs_h) {
            sy = (abs_h - h) / 2;
        }

        return std::make_pair(abs_x() + sx, abs_y() + sy);
    } else {
        int w = m_image->w() * m_image_scale;
        int h = m_image->h() * m_image_scale;
        int abs_w = this->abs_w();
        int abs_h = this->abs_h();
        int x = abs_w / 2  - (w / 2 + m_image_scroll_x); 
        int y = abs_h / 2  - (h / 2 + m_image_scroll_y);

        return std::make_pair(abs_x() + x,  abs_y() + y);
    }
}

float ImageViewer::compute_scale() {
    if (!m_fit_image) {
        return m_image_scale;
    }
    if (!m_image) {
        return 1.0;
    }
    
    auto abs_w = this->abs_w();
    auto abs_h = this->abs_h();
    
    if (abs_w == 0 || abs_h == 0 || m_image->w() == 0 || m_image->h() == 0) {
        return 0.0;
    }

    sf::Sprite sp(*m_texture);
        
    float scale_x = abs_w / (float) m_image->w();
    float scale_y = abs_h / (float) m_image->h();

    float result = 1.0;
    if (scale_x < scale_y) {
        result *= scale_x;
        int h = m_image->w() * scale_x;
        if (h > abs_h) {
            scale_y = (abs_h / (float) h);
            result *= scale_y;
        }
    } else {
        result *= scale_y;
        int w = m_image->h() * scale_y;
        if (w > abs_w) {
            scale_x = (abs_w / (float) w);
            result *= scale_x;
        }
    }

    return result;
}

void ImageViewer::paint(sf::RenderTarget *render_target) {
    if (!m_texture) return;
    float scale = compute_scale();
    sf::Sprite sp(*m_texture);
    auto origin = get_image_origin();
    sp.setPosition({origin.first, origin.second});
    sp.setScale({scale, scale});
    render_target->draw(sp);

}

void ImageViewer::image_scroll_x(int value) {
    m_image_scroll_x = value;
}

void ImageViewer::image_scroll_y(int value) {
    m_image_scroll_y = value;
}

int ImageViewer::image_scroll_x() {
    return m_image_scroll_x;
}

int ImageViewer::image_scroll_y() {
    return m_image_scroll_y;
}

void ImageViewer::handle_mouse_wheel(int8_t direction, int x, int y) {
    float zoom_inc = direction < 0 ? 0.10 : -0.10;
    m_image_scale += zoom_inc;
    if (m_image_scale < 0.10) {
        m_image_scale = 0.10;
        m_image_scroll_x = 0;
        m_image_scroll_y = 0;
    } else if (m_image_scale > 2.0) {
        m_image_scale = 2.0;
    }
}

void ImageViewer::handle_mouse_left_pressed(int x, int y)  {
    m_mouse_control = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RControl);
    m_mouse_pressed = true;
    m_mouse_down_x = x;
    m_mouse_down_y = y;
}

void ImageViewer::handle_mouse_left_released(int x, int y) {
    if (m_mouse_control) {
        int w = m_image->w() * m_image_scale;
        int h = m_image->h() * m_image_scale;
        if (m_image_scroll_x < - w * 0.5) {
            m_image_scroll_x = - w * 0.5;
        } else if (m_image_scroll_x > w * 0.5) {
            m_image_scroll_x =  w * 0.5;
        }
        if (m_image_scroll_y < - h * 0.5) {
            m_image_scroll_y = - h * 0.5;
        } else if (m_image_scroll_y > h * 0.5) {
            m_image_scroll_y =  h * 0.5;
        }
        m_mouse_control = false;
    }

}

component_cursor_t ImageViewer::cursor() {
    if (!m_fit_image && m_mouse_control && m_mouse_pressed) {
        return cursor_drag;
    }
    return Component::cursor();
};

bool ImageViewer::clickable() {
    return true;
}

void ImageViewer::handle_mouse_moved(int x, int y) {
    if (m_mouse_pressed && m_mouse_control && m_image) {
        auto distance_x = m_mouse_down_x - x;
        auto distance_y = m_mouse_down_y - y;
        m_mouse_down_x = x;
        m_mouse_down_y = y;
        m_image_scroll_x += distance_x;
        m_image_scroll_y += distance_y;
    }
}

} // namespace dfe_ui
