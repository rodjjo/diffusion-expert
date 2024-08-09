#include <SFML/Graphics.hpp>

#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"

#include "simple-ui/combobox.h"
#include "drawings.h"


namespace dfe_ui
{

Combobox::Combobox(Window * window, int x, int y, int w, int h) : Component(window) {
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_text_min_y();
    }
    text_color(theme::label_text_color());
    fill_color(theme::editor_fill_color());
    m_list.reset(new Listbox(window, x, y + h, w, h * 8));
    m_button.reset(new ArrowButton(window, w - h, 0, h, h, ArrowButton::arrow_down));
    Component::add(m_button);
    this->coordinates(x, y, w, h);
    m_button->onclick([this](Component *self) {
        show_list();
    });
    m_list->onclick([this](Component *self) {
        self->float_off();
    });
}

Combobox::~Combobox() {
}

void Combobox::show_list() {
    if (m_list->is_floatting()) {
        return;
    }
    m_list->coordinates(
        x(), y() + h() + 1, w(), m_list->h() 
    );
    m_list->float_on(parent());
}

void Combobox::paint(sf::RenderTarget *render_target) {
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

    txt.setFillColor(sf::Color(m_text_color));
    txt.setOutlineColor(sf::Color(m_text_color));

    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);
    dw.color(m_fill_color);
    int abs_h = this->abs_h();
    dw.size(abs_w() - abs_h, abs_h);
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);

    txt.setPosition({abs_x(), abs_y()});
    
    if (m_list->selected() < m_list->size()) {
        txt.setString(m_list->item(m_list->selected()));
        render_target->draw(txt);
    }
}

std::wstring Combobox::text() {

    return std::wstring();
}

int Combobox::character_size() {
    return m_character_size;
}

void Combobox::character_size(int value) {
    m_character_size = value;
}

void Combobox::text_color(uint32_t color) {
    m_text_color = color;
}

uint32_t Combobox::text_color() {
    return m_text_color;
}

void Combobox::fill_color(uint32_t color) {
    m_fill_color = color;
}

uint32_t Combobox::fill_color() {
    return m_fill_color;
}

void Combobox::update_text_min_y() {
}

void Combobox::handle_parent_resized() {
    m_button->coordinates(w() - h(), 0, h(), h());
    m_list->coordinates(abs_x(), abs_y() + w(), w(), (m_character_size  + theme::listbox_item_margin() * 2) * 5);
}

void Combobox::handle_mouse_wheel(int8_t direction, int x, int y) {

}

void Combobox::handle_mouse_left_pressed(int x, int y) {
    m_floatting_list = m_list->is_floatting();
};

void Combobox::handle_click() {
    if (m_list->is_floatting()) {
        return;
    }
    if (!m_floatting_list) {
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
    m_list->add(value);
}

void Combobox::remove(size_t index) {
    m_list->remove(index);
}

void Combobox::clear() {
    m_list->clear();
}

std::wstring Combobox::item(size_t index) {
    return m_list->item(index);
}

size_t Combobox::size() {
    return m_list->size();
}

    
} // namespace dfe_ui
 