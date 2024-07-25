#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/label.h"

namespace dfe_ui
{

Label::Label(int x, int y, int w, int h, const std::wstring &text) {
    this->coordinates(x, y, w, h);
    auto font = static_cast<sf::Font *>(load_default_font());
    if (font) {
        text_.reset(new sf::Text(*font));
        update_text_min_y();
        this->text(text);
    }
}

Label::~Label() {
}

void Label::paint(void *render_window) {
    if (!text_.get()) return;
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    
    int new_charsize = character_size_ * abs_scale();
    if (new_charsize < 1) {
        new_charsize = 1;
    }
    if (txt.getCharacterSize() != new_charsize) {
        txt.setCharacterSize(new_charsize);
        update_text_min_y();
    }

    int x = abs_x(), top = abs_y();
    int y = top - text_min_y_;

    int char_sz = txt.getCharacterSize();

     if (text_valign_ == text_alignment_middle)
        y += abs_h() / 2 - char_sz / 2;
    else if (text_valign_ == text_alligment_bottom)
        y += abs_h() - char_sz;

    txt.setPosition({(float)x, (float)y});
    txt.setFillColor(sf::Color(text_color_));
    txt.setOutlineColor(sf::Color(text_color_));
    static_cast<sf::RenderWindow *>(render_window)->draw(txt);
};

std::wstring Label::text() {
    if (text_) {
        return static_cast<sf::Text *>(text_.get())->getString();
    }
    return std::wstring();
}

int Label::character_size() {
    return character_size_;
}

void Label::character_size(int value) {
    character_size_ = value;
}

void Label::text(const std::wstring &value) {
    if (text_) {
        static_cast<sf::Text *>(text_.get())->setString(value);
    }
}

void Label::update_text_min_y() {
    if (text_) {
        text_min_y_ = compute_text_min_y(text_.get());
    }
}

void Label::text_color(uint32_t color) {
    text_color_ = color;
}

uint32_t Label::text_color() {
    return text_color_;
}


} // namespace dfe_ui
