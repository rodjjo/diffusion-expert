#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/label.h"


namespace dfe_ui
{

Label::Label(Window * window, int x, int y, int w, int h, const std::wstring &text) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_text_min_y();
        this->text(text);
    }
    text_color(theme::label_text_color());
}

Label::~Label() {
}

void Label::paint(sf::RenderTarget *render_target) {
    if (!m_text.get()) return;
    sf::Text &txt = *m_text.get();
    
    int new_charsize = m_character_size * abs_scale();
    if (new_charsize < 1) {
        new_charsize = 1;
    }
    if (txt.getCharacterSize() != new_charsize) {
        txt.setCharacterSize(new_charsize);
        update_text_min_y();
    }

    int x = abs_x(), top = abs_y();
    int y = top - m_text_min_y;

    int char_sz = txt.getCharacterSize();

     if (m_text_valign == text_alignment_middle)
        y += abs_h() / 2 - char_sz / 2;
    else if (m_text_valign == text_alligment_bottom)
        y += abs_h() - char_sz;

    txt.setPosition({(float)x, (float)y});
    txt.setFillColor(sf::Color(m_text_color));
    txt.setOutlineColor(sf::Color(m_text_color));
    render_target->draw(txt);
};

std::wstring Label::text() {
    if (m_text) {
        return static_cast<sf::Text *>(m_text.get())->getString();
    }
    return std::wstring();
}

int Label::character_size() {
    return m_character_size;
}

void Label::character_size(int value) {
    m_character_size = value;
}

void Label::text(const std::wstring &value) {
    if (m_text) {
        m_text->setString(value);
    }
}

void Label::update_text_min_y() {
    if (m_text) {
        m_text_min_y = compute_text_min_y(m_text.get());
    }
}

void Label::text_color(uint32_t color) {
    m_text_color = color;
}

uint32_t Label::text_color() {
    return m_text_color;
}


} // namespace dfe_ui
