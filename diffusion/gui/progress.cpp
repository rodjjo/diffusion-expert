#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/progress.h"

namespace dfe_ui
{

ProgressBar::ProgressBar(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_text_min_y();
    }
    text_color(theme::progress_bar_text_color());
    fill_color(theme::progress_bar_color());
    outline_color(theme::progress_bar_outline_color());
    progress_color(theme::progress_bar_progress_color());
}

ProgressBar::~ProgressBar() {
}

int64_t ProgressBar::progress() {
    return m_progress;
}

void ProgressBar::progress(int64_t value) {
    if (value > m_progress_max)  {
        value = m_progress_max;
    }
    if (value < 0) {
        value = 0;
    }
    m_progress = value;
}

int ProgressBar::progress_max(int64_t value) {
    if (value < 1) {
        value = 1;
    }
    m_progress_max = value;
}

int64_t ProgressBar::progress_max() {
    return m_progress_max;
}

int ProgressBar::character_size() {
    return m_character_size;
}

void ProgressBar::character_size(int value) {
    m_character_size = value;
}

void ProgressBar::text_color(uint32_t color) {
    m_text_color = color;
}

uint32_t ProgressBar::text_color() {
    return m_text_color;
}

void ProgressBar::paint(sf::RenderTarget *render_target) {
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

    char buffer[100] = "";
    if (m_progress_max < 1) 
        m_progress_max = 1;

    float progress = static_cast<float>(m_progress * (100.0 / m_progress_max));
    sprintf(buffer, "%0.0f %%", progress);

    if (txt.getString() != buffer) {
        txt.setString(buffer);
    }

    int x = abs_x(), top = abs_y();
    int y = top - m_text_min_y;

    x += (abs_w() / 2) - (txt.getLocalBounds().size.y / 2);
    int char_sz = txt.getCharacterSize();
    y += abs_h() / 2 - char_sz / 2;


    sf::RectangleShape rect(sf::Vector2f(abs_w(), abs_h()));
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(m_fill_color));
    rect.setOutlineColor(sf::Color(m_outline_color));
    render_target->draw(rect);

    rect.setSize({(abs_w() / 100.0) * progress, abs_h()});
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(m_progress_color));
    rect.setOutlineColor(sf::Color(m_outline_color));
    render_target->draw(rect);

    txt.setPosition({(float)x, (float)y});
    txt.setFillColor(sf::Color(m_text_color));
    txt.setOutlineColor(sf::Color(m_text_color));
    render_target->draw(txt);

};

void ProgressBar::fill_color(uint32_t color) {
    m_fill_color = color;
}

uint32_t ProgressBar::fill_color() {
    return m_fill_color;
}

void ProgressBar::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t ProgressBar::outline_color() {
    return m_outline_color;
}

void ProgressBar::progress_color(uint32_t color) {
    m_progress_color = color;
}

uint32_t ProgressBar::progress_color() {
    return m_progress_color;
}

void ProgressBar::update_text_min_y() {
    if (m_text) {
        m_text_min_y = compute_text_min_y(m_text.get());
    }
}


} // namespace def_ui
