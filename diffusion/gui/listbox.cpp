#include <SFML/Graphics.hpp>

#include "drawings.h"
#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"
#include "simple-ui/listbox.h"


namespace dfe_ui {


Listbox::Listbox(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_text_min_y();
    }
    text_color(theme::label_text_color());
    fill_color(theme::progress_bar_color());
    outline_color(theme::progress_bar_outline_color());
    selected_color(theme::listbox_selected_color());
    selected_text_color(theme::listbox_selected_text_color());
    m_scrollbar.reset(new Scrollbar(window, w - 25, 0, 25, h, true));
    Component::add(m_scrollbar);
    m_scrollbar->visible(false);
    m_scrollbar->onchange([this](Component *self) {
        scrollbar_changed();
    });
}

Listbox::~Listbox() {

}

std::wstring Listbox::item(size_t index) {
    return m_items[index];
}

size_t Listbox::size() {
    return m_items.size();
}
 
void Listbox::update_text_min_y() {
    if (m_text) {
        m_text_min_y = compute_text_min_y(m_text.get());
    }
}

int Listbox::character_size() {
    return m_character_size;
}

void Listbox::character_size(int value) {
    m_character_size = value;
}

void Listbox::text_color(uint32_t color) {
    m_text_color = color;
}

uint32_t Listbox::text_color() {
    return m_text_color;
}

void Listbox::fill_color(uint32_t value) {
    m_fill_color = value;
}

uint32_t Listbox::fill_color() {
    return m_fill_color;
}

void Listbox::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t Listbox::outline_color() {
    return m_outline_color;
}

void Listbox::paint(sf::RenderTarget *render_target) {
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

    // draw the selected item
    int64_t selected_on_display = m_selected_index - m_top_element;
    int visible_items = this->visible_items();
    int item_height = this->item_height();

    int w = abs_w();

    if (m_scrollbar->visible()) {
        w -= m_scrollbar->w();
    }

    // draw the box
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);
    dw.color(m_fill_color);
    dw.size(w, abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);


    if (m_selected_index < m_items.size() && selected_on_display >= 0 && selected_on_display < visible_items) {
        dw.outline_color(m_selected_color);
        dw.color(m_selected_color);
        dw.size(w, item_height);
        dw.position(abs_x(), abs_y() + selected_on_display * item_height);
        dw.draw(render_target);    
    }

    int x = abs_x(), top = abs_y();
    int y = top - m_text_min_y;
    int char_sz = txt.getCharacterSize();
    y += item_height / 2 - char_sz / 2;

    render_target->draw(txt);
    for (int i = 0; i < visible_items; i++) {
        if (i + m_top_element >= m_items.size()) break;
        txt.setString(m_items[i + m_top_element]);
        txt.setPosition({(float)x, (float)y + item_height * i});
        if (i == selected_on_display) {
            txt.setFillColor(sf::Color(m_selected_text_color));
            txt.setOutlineColor(sf::Color(m_selected_text_color));
        } else {
            txt.setFillColor(sf::Color(m_text_color));
            txt.setOutlineColor(sf::Color(m_text_color));
        }
        render_target->draw(txt);
    }
};

void Listbox::add(const std::wstring& value) {
    m_items.push_back(value);
    if (m_items.size() > visible_items()) {
        m_scrollbar->visible(true);
    }
    update_scrollbar();
}

void Listbox::scrollbar_changed() {
    if (m_changing_scrollbar) return;
    if (m_items.size() < 1) {
        m_top_element = 0;
        m_selected_index = 0;
        return;
    }
    m_top_element = m_scrollbar->value();
}

void Listbox::remove(size_t index) {
    m_items.erase(m_items.begin() + index);
    if (selected() > 0 && selected() >= m_items.size()) {
        selected(selected() - 1);
    }
    update_scrollbar();
}

void Listbox::clear() {
    m_items.clear();
    m_selected_index = 0;
    m_top_element = 0;
    m_scrollbar->visible(false);
    update_scrollbar();
}

int Listbox::item_height() {
    if (!m_text.get()) return 1;
    sf::Text &txt = *m_text.get();
    return txt.getCharacterSize() + (theme::listbox_item_margin() * 2) * abs_scale();
}

int Listbox::visible_items() {
    int ih = item_height();
    if (ih == 0) return 1;
    return abs_h() / ih;
}

void Listbox::selected_text_color(int32_t color) {
    m_selected_text_color = color;
}

uint32_t Listbox::selected_text_color() {
    return m_selected_text_color;
}

void Listbox::selected_color(int32_t color) {
    m_selected_color = color;
}

uint32_t Listbox::selected_color() {
    return m_selected_color;
}

void Listbox::handle_mouse_left_pressed(int x, int y) {
    if (!m_text.get()) return;
    float scale = abs_scale();
    if (scale == 0) {
        return;
    }
    int item_height = this->item_height() / scale;
    int item_count = y / item_height;
    selected(m_top_element + item_count);
    if (m_onclick) {
        m_onclick(this);
    }
}

void Listbox::update_scrollbar() {
    if (m_changing_scrollbar) return;
    m_changing_scrollbar = true;
    m_scrollbar->min(0);
    m_scrollbar->max(m_items.size() - visible_items());
    m_scrollbar->value(m_top_element);
    m_changing_scrollbar = false;
}

void Listbox::selected(size_t value) {
    if (value >= 0 && value < m_items.size()) {
        this->m_selected_index = value;
        size_t visible_items = this->visible_items();
        if (value < m_top_element) {
            m_top_element = value;
        } else if (value + 1 > m_top_element + visible_items) {
            m_top_element = (value - visible_items) + 1;
        }
        update_scrollbar();
        if (m_onchange) {
            m_onchange(this);
        }
    }
}

size_t Listbox::selected() {
    return this->m_selected_index;
}

void Listbox::handle_mouse_wheel(int8_t direction, int x, int y) {
    if (direction < 0) {
        m_scrollbar->value(m_scrollbar->value() + 1);
    } else {
        m_scrollbar->value(m_scrollbar->value() - 1);
    }
}

bool Listbox::clickable() {
    return true;
}

bool Listbox::focusable() {
    return true;
}

void Listbox::scrollbar_width(int value) {
    m_scrollbar->w(value);
}

int Listbox::scrollbar_width() {
    return m_scrollbar->w();
}

void Listbox::onchange(component_event_t value) {
    m_onchange = value;
}

component_event_t Listbox::onchange() {
    return m_onchange;
}

void Listbox::onclick(component_event_t value) {
    m_onclick = value;
}

component_event_t Listbox::onclick() {
    return m_onclick;
}

void Listbox::handle_parent_resized() {
    if (m_scrollbar) {
        m_scrollbar->x(this->w() - m_scrollbar->w());
        m_scrollbar->y(0);
        m_scrollbar->h(this->h());
    }
}

}  // namespace dfe_ui
