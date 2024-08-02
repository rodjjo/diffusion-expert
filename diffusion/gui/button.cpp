#include <SFML/Graphics.hpp>

#include "simple-ui/icons.h"
#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/button.h"
#include "simple-ui/win.h"
#include "drawings.h"

namespace dfe_ui
{

Button::Button(Window * window, int x, int y, int w, int h, const std::wstring &text, icon_type_t icon_type) : Component(window), m_icon_type(icon_type) {
    this->coordinates(x, y, w, h);
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_text_min_y();
        this->text(text);
    }
    text_color(dfe_ui::theme::button_text_color());
    color(dfe_ui::theme::button_fill_color());
    outline_color(dfe_ui::theme::button_outline_color());
    highlighted_color(dfe_ui::theme::button_highlighted_color());
    pressed_color(dfe_ui::theme::button_pressed_color());
}

void Button::update_text_min_y() {
    if (m_text) {
        m_text_min_y = compute_text_min_y(m_text.get());
    }
}

Button::~Button() {
}

std::wstring Button::text() {
    if (m_text) {
        return static_cast<sf::Text *>(m_text.get())->getString();
    }
    return std::wstring();
}

void Button::text(const std::wstring &value) {
    if (m_text) {
        static_cast<sf::Text *>(m_text.get())->setString(value);
    }
}

int Button::character_size() {
    return m_character_size;
}

void Button::character_size(int value) {
    m_character_size = value;
}

void Button::text_color(uint32_t color) {
    m_text_color = color;
}

uint32_t Button::text_color() {
    return m_text_color;
}


void Button::color(uint32_t value) {
    m_color = value;
}

uint32_t Button::color() {
    return m_color;
}

void Button::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t Button::outline_color() {
    return m_outline_color;
}

Button::icon_position_t Button::icon_position() {
    return m_icon_pos;
}

void Button::highlighted_color(uint32_t color) {
    m_highlighted_color = color;
}

uint32_t Button::highlighted_color() {
    return m_highlighted_color;
}

void Button::handle_mouse_left_pressed(int x, int y) {
    m_mouse_pressed = true;
}

void Button::handle_mouse_left_released(int x, int y) {
    m_mouse_pressed = false;
}

void Button::handle_click() {
    if (m_checked_icon != img_none) {
        m_checked = !m_checked;
    }
    if (m_on_click) {
        m_on_click(this);
    }
}

uint32_t Button::pressed_color() {
    return m_pressed_color;
}

void Button::pressed_color(uint32_t value) {
    m_pressed_color = value;
}


void Button::icon_position(Button::icon_position_t value) {
    m_icon_pos = value;
}

void Button::mouse_enter() {
    m_mouse_inside = true;
}

void Button::mouse_exit() {
    m_mouse_inside = false;
}

bool Button::clickable() {
    return true;
}

component_cursor_t Button::cursor() {
    return cursor_hand;
}

void Button::checked_icon(icon_type_t icon_type) {
    m_checked_icon = icon_type;
}

icon_type_t Button::checked_icon() {
    return m_checked_icon;
}

bool Button::checked() {
    return m_checked;
}

void Button::checked(bool value) {
    m_checked = value;
}

void Button::paint(sf::RenderTarget *render_target) {
    if (!m_text) return;
    auto txt = *static_cast<sf::Text *>(m_text.get());
    
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);

    if (m_mouse_pressed) {
        dw.color(m_pressed_color);
    } else if (m_mouse_inside) {
        dw.color(m_highlighted_color);
    } else {
        dw.color(m_color);  
    }
    
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);

    auto scale = abs_scale();
    IconTextureBase *icons = NULL;
    std::pair<int, int> icon_coord(0, 0);
    std::pair<int, int> icon_size(0, 0);

    if (m_icon_type != img_none) {
        auto icon = m_checked && (m_checked_icon != img_none) ? m_checked_icon :  m_icon_type;
        icons = load_icons_texture();
        icon_coord = icons->get_coords(icon);
        icon_size = icons->get_size(icon);
    }

    if (!txt.getString().isEmpty()) {
        int new_charsize = m_character_size * abs_scale();
        if (new_charsize < 1) {
            new_charsize = 1;
        }
        if (txt.getCharacterSize() != new_charsize) {
            txt.setCharacterSize(new_charsize);
            update_text_min_y();
        }

        int text_w = txt.getLocalBounds().size.x;
        int x = abs_x() + abs_w() / 2 - text_w / 2, top = abs_y();
        int y = top - m_text_min_y;
        int icon_h =  (icon_size.second / 2) * scale;
        int char_sz = txt.getCharacterSize();
    

        y += abs_h() / 2 - char_sz / 2 + (m_icon_pos == icon_center && icons != NULL ? icon_h / 2 : 0 );

        if (icons) {
            sf::Sprite icon(*static_cast<sf::Texture *>(icons->sfml_texture()), {{icon_coord.first, icon_coord.second}, { icon_size.first, icon_size.second}});
            icon.setScale({abs_scale(), abs_scale()});
            if (m_icon_pos == icon_center) {
                icon.setPosition({abs_x() + abs_w() / 2 - (icon_size.first / 2) * scale, y - icon_size.second * scale});
            } else if (m_icon_pos == icon_left) { 
                icon.setPosition({x - icon_size.first * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else if (m_icon_pos == icon_left_most) { 
                icon.setPosition({abs_x() + theme::icon_margin() * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else if (m_icon_pos == icon_right_most) { 
                icon.setPosition({abs_x() + abs_w() - icon_size.first * scale - theme::icon_margin() * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else { 
                icon.setPosition({x + text_w, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            }
            render_target->draw(icon);
        }

        txt.setPosition({(float)x, (float)y});
        txt.setFillColor(sf::Color(m_text_color));
        txt.setOutlineColor(sf::Color(m_text_color));
        render_target->draw(txt);
    } else {
        if (icons) {
            sf::Sprite icon(*static_cast<sf::Texture *>(icons->sfml_texture()), {{icon_coord.first, icon_coord.second}, { icon_size.first, icon_size.second}});
            icon.setScale({abs_scale(), abs_scale()});
            icon.setPosition({abs_x() + abs_w() / 2 - (icon_size.first / 2) * scale, abs_y() + abs_h() / 2 - (icon_size.second / 2) * scale});
            render_target->draw(icon);
        }
    }
}

void Button::onclick(cb_button_click_t value) {
    m_on_click = value;
}



    
} // namespace dfe_ui
