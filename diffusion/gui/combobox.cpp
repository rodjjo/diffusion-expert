#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"

#include "simple-ui/combobox.h"
#include "drawings.h"


namespace dfe_ui
{

Combobox::Combobox(Window * window, int x, int y, int w, int h) : Component(window) {
    auto font = static_cast<sf::Font *>(load_default_font());
    if (font) {
        text_.reset(new sf::Text(*font));
        update_text_min_y();
    }
    text_color(theme::label_text_color());
    fill_color(theme::editor_fill_color());
    list_.reset(new Listbox(window, x, y + h, w, h * 8));
    button_.reset(new ArrowButton(window, w - h, 0, h, h, ArrowButton::arrow_down));
    Component::add(button_);
    this->coordinates(x, y, w, h);
    button_->onclick([this](Component *self) {
        show_list();
    });
    list_->onclick([this](Component *self) {
        self->float_off();
    });
}

Combobox::~Combobox() {
}

void Combobox::show_list() {
    if (list_->is_floatting()) {
        return;
    }
    list_->coordinates(
        x(), y() + h() + 1, w(), list_->h() 
    );
    list_->float_on(parent());
}

void Combobox::paint(void *render_window) {
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

    txt.setFillColor(sf::Color(text_color_));
    txt.setOutlineColor(sf::Color(text_color_));

    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(outline_color_);
    dw.color(fill_color_);
    int abs_h = this->abs_h();
    dw.size(abs_w() - abs_h, abs_h);
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_window);

    txt.setPosition({abs_x(), abs_y()});
    
    if (list_->selected() < list_->size()) {
        txt.setString(list_->at(list_->selected()));
        static_cast<sf::RenderWindow *>(render_window)->draw(txt);
    }
}

std::wstring Combobox::text() {

    return std::wstring();
}

int Combobox::character_size() {
    return character_size_;
}

void Combobox::character_size(int value) {
    character_size_ = value;
}

void Combobox::text_color(uint32_t color) {
    text_color_ = color;
}

uint32_t Combobox::text_color() {
    return text_color_;
}

void Combobox::fill_color(uint32_t color) {
    fill_color_ = color;
}

uint32_t Combobox::fill_color() {
    return fill_color_;
}

void Combobox::update_text_min_y() {
}

void Combobox::handle_parent_resized() {
    button_->coordinates(w() - h(), 0, h(), h());
    list_->coordinates(abs_x(), abs_y() + w(), w(), (character_size_  + theme::listbox_item_margin() * 2) * 5);
}

void Combobox::handle_mouse_wheel(int8_t direction, int x, int y) {

}

void Combobox::handle_mouse_left_pressed(int x, int y) {
    floatting_list_ = list_->is_floatting();
};

void Combobox::handle_click() {
    if (list_->is_floatting()) {
        return;
    }
    if (!floatting_list_) {
        show_list();
    }
}

bool Combobox::clickable() {
    return true;
}

bool Combobox::focusable() {
    return true;
}

void Combobox::add(const std::wstring& value) {
    list_->add(value);
}

void Combobox::remove(size_t index) {
    list_->remove(index);
}

void Combobox::clear() {
    list_->clear();
}

std::wstring Combobox::at(size_t index) {
    return list_->at(index);
}

size_t Combobox::size() {
    return list_->size();
}

    
} // namespace dfe_ui
 