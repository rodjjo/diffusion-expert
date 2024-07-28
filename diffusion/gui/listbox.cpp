#include <SFML/Graphics.hpp>

#include "drawings.h"
#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/listbox.h"


namespace dfe_ui {


Listbox::Listbox(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = static_cast<sf::Font *>(load_default_font());
    if (font) {
        text_.reset(new sf::Text(*font));
        update_text_min_y();
    }
    text_color(theme::label_text_color());
    fill_color(theme::progress_bar_color());
    outline_color(theme::progress_bar_outline_color());
    selected_color(theme::listbox_selected_color());
    selected_text_color(theme::listbox_selected_text_color());
    scrollbar_.reset(new Scrollbar(window, w - 25, 0, 25, h, true));
    Component::add(scrollbar_);
    scrollbar_->visible(false);
}

Listbox::~Listbox() {

}

std::wstring Listbox::at(size_t index) {
    return items_[index];
}

size_t Listbox::size() {
    return items_.size();
}
 
void Listbox::update_text_min_y() {
    if (text_) {
        text_min_y_ = compute_text_min_y(text_.get());
    }
}

int Listbox::character_size() {
    return character_size_;
}

void Listbox::character_size(int value) {
    character_size_ = value;
}

void Listbox::text_color(uint32_t color) {
    text_color_ = color;
}

uint32_t Listbox::text_color() {
    return text_color_;
}

void Listbox::fill_color(uint32_t value) {
    fill_color_ = value;
}

uint32_t Listbox::fill_color() {
    return fill_color_;
}

void Listbox::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t Listbox::outline_color() {
    return outline_color_;
}

void Listbox::paint(void *render_window) {
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

    // draw the selected item
    int64_t selected_on_display = selected_index_ - top_element_;
    int visible_items = this->visible_items();
    int item_height = this->item_height();

    int w = abs_w();

    if (scrollbar_->visible()) {
        w -= scrollbar_->w();
    }

    // draw the box
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(outline_color_);
    dw.color(fill_color_);
    dw.size(w, abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_window);


    if (selected_index_ < items_.size() && selected_on_display >= 0 && selected_on_display < visible_items) {
        dw.outline_color(selected_color_);
        dw.color(selected_color_);
        dw.size(w, item_height);
        dw.position(abs_x(), abs_y() + selected_on_display * item_height);
        dw.draw(render_window);    
    }

    int x = abs_x(), top = abs_y();
    int y = top - text_min_y_;
    int char_sz = txt.getCharacterSize();
    y += item_height / 2 - char_sz / 2;

    static_cast<sf::RenderWindow *>(render_window)->draw(txt);
    for (int i = 0; i < visible_items; i++) {
        if (i + top_element_ >= items_.size()) break;
        txt.setString(items_[i + top_element_]);
        txt.setPosition({(float)x, (float)y + item_height * i});
        if (i == selected_on_display) {
            txt.setFillColor(sf::Color(selected_text_color_));
            txt.setOutlineColor(sf::Color(selected_text_color_));
        } else {
            txt.setFillColor(sf::Color(text_color_));
            txt.setOutlineColor(sf::Color(text_color_));
        }
        static_cast<sf::RenderWindow *>(render_window)->draw(txt);
    }
};

void Listbox::add(const std::wstring& value) {
    items_.push_back(value);
    if (items_.size() > visible_items()) {
        scrollbar_->visible(true);
    }
}

void Listbox::remove(size_t index) {
    items_.erase(items_.begin() + index);
}

void Listbox::clear() {
    items_.clear();
    scrollbar_->visible(false);
}

int Listbox::item_height() {
    if (!text_.get()) return 1;
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    return txt.getCharacterSize() + (theme::listbox_item_margin() * 2) * abs_scale();
}

int Listbox::visible_items() {
    int ih = item_height();
    if (ih == 0) return 1;
    return abs_h() / ih;
}

void Listbox::selected_text_color(int32_t color) {
    selected_text_color_ = color;
}

uint32_t Listbox::selected_text_color() {
    return selected_text_color_;
}

void Listbox::selected_color(int32_t color) {
    selected_color_ = color;
}

uint32_t Listbox::selected_color() {
    return selected_color_;
}

void Listbox::handle_mouse_left_pressed(int x, int y) {
    if (!text_.get()) return;
    int item_height = this->item_height();
    int item_count = y / item_height;
    selected(top_element_ + item_count);
}

void Listbox::selected(size_t value) {
    if (value >= 0 && value < items_.size()) {
        this->selected_index_ = value;
        size_t visible_items = this->visible_items();
        if (value < top_element_) {
            top_element_ = value;
        } else if (value + 1 > top_element_ + visible_items) {
            top_element_ = (value - visible_items) + 1;
        }
    }
}

size_t Listbox::selected() {
    return this->selected_index_;
}

void Listbox::handle_mouse_wheel(int8_t direction, int x, int y) {
    if (direction < 0) {
        selected(selected() +1);
    } else {
        selected(selected() -1);
    }
}

bool Listbox::clickable() {
    return true;
}

bool Listbox::focusable() {
    return true;
}

void Listbox::scrollbar_width(int value) {
    scrollbar_->w(value);
}

int Listbox::scrollbar_width() {
    return scrollbar_->w();
}

void Listbox::handle_parent_resized() {
    if (scrollbar_) {
        scrollbar_->x(this->w() - scrollbar_->w());
        scrollbar_->y(0);
        scrollbar_->h(this->h());
    }
}

}  // namespace dfe_ui
