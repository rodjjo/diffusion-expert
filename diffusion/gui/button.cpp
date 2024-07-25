#include <SFML/Graphics.hpp>

#include "simple-ui/icons.h"
#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/button.h"
#include "drawings.h"

namespace dfe_ui
{

Button::Button(int x, int y, int w, int h, const std::wstring &text, icon_type_t icon_type) : Component(), icon_type_(icon_type) {
    this->coordinates(x, y, w, h);
    auto font = static_cast<sf::Font *>(load_default_font());
    if (font) {
        text_.reset(new sf::Text(*font));
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
    if (text_) {
        text_min_y_ = compute_text_min_y(text_.get());
    }
}

Button::~Button() {
}

std::wstring Button::text() {
    if (text_) {
        return static_cast<sf::Text *>(text_.get())->getString();
    }
    return std::wstring();
}

void Button::text(const std::wstring &value) {
    if (text_) {
        static_cast<sf::Text *>(text_.get())->setString(value);
    }
}

int Button::character_size() {
    return character_size_;
}

void Button::character_size(int value) {
    character_size_ = value;
}

void Button::text_color(uint32_t color) {
    text_color_ = color;
}

uint32_t Button::text_color() {
    return text_color_;
}


void Button::color(uint32_t value) {
    color_ = value;
}

uint32_t Button::color() {
    return color_;
}

void Button::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t Button::outline_color() {
    return outline_color_;
}

Button::icon_position_t Button::icon_position() {
    return icon_pos_;
}

void Button::highlighted_color(uint32_t color) {
    highlighted_color_ = color;
}

uint32_t Button::highlighted_color() {
    return highlighted_color_;
}

void Button::handle_mouse_left_pressed(int x, int y) {
    mouse_pressed_ = true;
}

void Button::handle_mouse_left_released(int x, int y) {
    mouse_pressed_ = false;
    if (checked_icon_ != img_none) {
        checked_ = !checked_;
    }
}

uint32_t Button::pressed_color() {
    return pressed_color_;
}

void Button::pressed_color(uint32_t value) {
    pressed_color_ = value;
}


void Button::icon_position(Button::icon_position_t value) {
    icon_pos_ = value;
}

void Button::mouse_enter() {
    mouse_inside_ = true;
}

void Button::mouse_exit() {
    mouse_inside_ = false;
}

bool Button::clickable() {
    return true;
}

component_cursor_t Button::cursor() {
    return cursor_hand;
}

void Button::checked_icon(icon_type_t icon_type) {
    checked_icon_ = icon_type;
}

icon_type_t Button::checked_icon() {
    return checked_icon_;
}

bool Button::checked() {
    return checked_;
}

void Button::checked(bool value) {
    checked_ = value;
}

void Button::paint(void *render_window) {
    if (!text_) return;
    auto txt = *static_cast<sf::Text *>(text_.get());
    
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(outline_color_);

    if (mouse_pressed_) {
        dw.color(pressed_color_);
    } else if (mouse_inside_) {
        dw.color(highlighted_color_);
    } else {
        dw.color(color_);  
    }
    
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_window);

    auto scale = abs_scale();
    IconTextureBase *icons = NULL;
    std::pair<int, int> icon_coord(0, 0);
    std::pair<int, int> icon_size(0, 0);

    if (icon_type_ != img_none) {
        auto icon = checked_ && (checked_icon_ != img_none) ? checked_icon_ :  icon_type_;
        icons = load_icons_texture();
        icon_coord = icons->get_coords(icon);
        icon_size = icons->get_size(icon);
    }

    if (!txt.getString().isEmpty()) {
        int new_charsize = character_size_ * abs_scale();
        if (new_charsize < 1) {
            new_charsize = 1;
        }
        if (txt.getCharacterSize() != new_charsize) {
            txt.setCharacterSize(new_charsize);
            update_text_min_y();
        }

        int text_w = txt.getLocalBounds().size.x;
        int x = abs_x() + abs_w() / 2 - text_w / 2, top = abs_y();
        int y = top - text_min_y_;
        int icon_h =  (icon_size.second / 2) * scale;
        int char_sz = txt.getCharacterSize();
    

        y += abs_h() / 2 - char_sz / 2 + (icon_pos_ == icon_center && icons != NULL ? icon_h / 2 : 0 );

        if (icons) {
            sf::Sprite icon(*static_cast<sf::Texture *>(icons->sfml_texture()), {{icon_coord.first, icon_coord.second}, { icon_size.first, icon_size.second}});
            icon.setScale({abs_scale(), abs_scale()});
            if (icon_pos_ == icon_center) {
                icon.setPosition({abs_x() + abs_w() / 2 - (icon_size.first / 2) * scale, y - icon_size.second * scale});
            } else if (icon_pos_ == icon_left) { 
                icon.setPosition({x - icon_size.first * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else if (icon_pos_ == icon_left_most) { 
                icon.setPosition({abs_x() + theme::icon_margin() * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else if (icon_pos_ == icon_right_most) { 
                icon.setPosition({abs_x() + abs_w() - icon_size.first * scale - theme::icon_margin() * scale, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            } else { 
                icon.setPosition({x + text_w, abs_y() + abs_h() / 2  - (icon_size.second / 2) * scale });
            }
            static_cast<sf::RenderWindow *>(render_window)->draw(icon);
        }

        txt.setPosition({(float)x, (float)y});
        txt.setFillColor(sf::Color(text_color_));
        txt.setOutlineColor(sf::Color(text_color_));
        static_cast<sf::RenderWindow *>(render_window)->draw(txt);
    } else {
        if (icons) {
            sf::Sprite icon(*static_cast<sf::Texture *>(icons->sfml_texture()), {{icon_coord.first, icon_coord.second}, { icon_size.first, icon_size.second}});
            icon.setScale({abs_scale(), abs_scale()});
            icon.setPosition({abs_x() + abs_w() / 2 - (icon_size.first / 2) * scale, abs_y() + abs_h() / 2 - (icon_size.second / 2) * scale});
            static_cast<sf::RenderWindow *>(render_window)->draw(icon);
        }
    }
}



    
} // namespace dfe_ui
