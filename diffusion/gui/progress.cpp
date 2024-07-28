#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/progress.h"

namespace dfe_ui
{

ProgressBar::ProgressBar(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = static_cast<sf::Font *>(load_default_font());
    if (font) {
        text_.reset(new sf::Text(*font));
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
    return progress_;
}

void ProgressBar::progress(int64_t value) {
    if (value > progress_max_)  {
        value = progress_max_;
    }
    if (value < 0) {
        value = 0;
    }
    progress_ = value;
}

int ProgressBar::progress_max(int64_t value) {
    if (value < 1) {
        value = 1;
    }
    progress_max_ = value;
}

int64_t ProgressBar::progress_max() {
    return progress_max_;
}

int ProgressBar::character_size() {
    return character_size_;
}

void ProgressBar::character_size(int value) {
    character_size_ = value;
}

void ProgressBar::text_color(uint32_t color) {
    text_color_ = color;
}

uint32_t ProgressBar::text_color() {
    return text_color_;
}

void ProgressBar::paint(void *render_window) {
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

    char buffer[100] = "";
    if (progress_max_ < 1) 
        progress_max_ = 1;

    float progress = static_cast<float>(progress_ * (100.0 / progress_max_));
    sprintf(buffer, "%0.0f %%", progress);

    if (txt.getString() != buffer) {
        txt.setString(buffer);
    }

    int x = abs_x(), top = abs_y();
    int y = top - text_min_y_;

    x += (abs_w() / 2) - (txt.getLocalBounds().size.y / 2);
    int char_sz = txt.getCharacterSize();
    y += abs_h() / 2 - char_sz / 2;


    sf::RectangleShape rect(sf::Vector2f(abs_w(), abs_h()));
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(fill_color_));
    rect.setOutlineColor(sf::Color(outline_color_));
    static_cast<sf::RenderWindow *>(render_window)->draw(rect);

    rect.setSize({(abs_w() / 100.0) * progress, abs_h()});
    rect.setPosition(sf::Vector2f(abs_x(), abs_y()));
    rect.setFillColor(sf::Color(progress_color_));
    rect.setOutlineColor(sf::Color(outline_color_));
    static_cast<sf::RenderWindow *>(render_window)->draw(rect);

    txt.setPosition({(float)x, (float)y});
    txt.setFillColor(sf::Color(text_color_));
    txt.setOutlineColor(sf::Color(text_color_));
    static_cast<sf::RenderWindow *>(render_window)->draw(txt);

};

void ProgressBar::fill_color(uint32_t color) {
    fill_color_ = color;
}

uint32_t ProgressBar::fill_color() {
    return fill_color_;
}

void ProgressBar::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t ProgressBar::outline_color() {
    return outline_color_;
}

void ProgressBar::progress_color(uint32_t color) {
    progress_color_ = color;
}

uint32_t ProgressBar::progress_color() {
    return progress_color_;
}

void ProgressBar::update_text_min_y() {
    if (text_) {
        text_min_y_ = compute_text_min_y(text_.get());
    }
}


} // namespace def_ui
