#include <stdio.h>
#include "simple-ui/theme.h"
#include "simple-ui/scrollbox.h"
#include "drawings.h"

namespace dfe_ui {


Scrollbox::Scrollbox(Window * window, int x, int y, int w, int h) : Component(window) {
    m_scroll_component.reset(new Component(window));
    m_scroll_component->coordinates(0, 0, w, h);
    this->coordinates(x, y, w, h);
    m_vertical_sb.reset(new Scrollbar(window, w, 0, w - theme::scrollbox_scrollbar_size(), h, true));
    m_horizontal_sb.reset(new Scrollbar(window, 0, h - theme::scrollbox_scrollbar_size(), w - theme::scrollbox_scrollbar_size(), theme::scrollbox_scrollbar_size(), false));
    m_horizontal_sb->onchange([this](Component *) {
        sync_scrollbar();
    });
    m_vertical_sb->onchange([this](Component *) {
        sync_scrollbar();
    });
    m_vertical_sb->visible(false);
    m_horizontal_sb->visible(false);
    Component::add(m_scroll_component);
    Component::add(m_vertical_sb);
    Component::add(m_horizontal_sb);
    handle_parent_resized();
    fill_color(theme::scrollbox_color());
    outline_color(theme::scrollbox_outline_color());
}

Scrollbox::~Scrollbox() {
}

uint32_t Scrollbox::fill_color() {
    return m_fill_color;
}

void Scrollbox::fill_color(uint32_t value) {
    m_fill_color = value;
}

void Scrollbox::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t Scrollbox::outline_color() {
    return m_outline_color;
}

bool Scrollbox::autoscroll() {
    return m_autoscroll;
}

void Scrollbox::autoscroll(bool value) {
    m_autoscroll = value;
}

void Scrollbox::scroll_x(int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > m_max_scroll_x) {
        value = m_max_scroll_x;
    }
    m_scroll_component->scroll_x(value);
}

void Scrollbox::scroll_y(int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > m_max_scroll_y) {
        value = m_max_scroll_y;
    }
    m_scroll_component->scroll_y(value);
}

void Scrollbox::add(std::shared_ptr<Component> child) {
    m_scroll_component->add(child);
    compute_autoscroll();
};

Component & Scrollbox::scroll_component() {
    return *m_scroll_component.get();
};


void Scrollbox::handle_parent_resized() {
    if (!m_vertical_sb) return;
    if (!m_horizontal_sb) return;
    m_horizontal_sb->coordinates(0, h() - theme::scrollbox_scrollbar_size(), w() - theme::scrollbox_scrollbar_size(), theme::scrollbox_scrollbar_size());
    m_vertical_sb->coordinates(w() - theme::scrollbox_scrollbar_size(), 0, theme::scrollbox_scrollbar_size(), h() - theme::scrollbox_scrollbar_size());
    compute_autoscroll();
    positionate_scrollbars();
};

void Scrollbox::positionate_scrollbars() {
    int w = this->w();
    int h = this->h();
    if (m_horizontal_sb->visible()) {
        h = m_horizontal_sb->y();
    }
    if (m_vertical_sb->visible()) {
        w = m_vertical_sb->x();
    }
    m_scroll_component->coordinates(0, 0, w, h);
}

void Scrollbox::handle_child_count_changed() {
    compute_autoscroll();
};

void Scrollbox::compute_autoscroll() {
    if (!m_autoscroll) return;
    if (!m_scroll_component) return;
    int mx = 0;
    int my = 0;
    for (size_t i = 0; i < m_scroll_component->component_count(); i++) {
        if (mx < m_scroll_component->at(i).x()) {
            mx = m_scroll_component->at(i).x() + m_scroll_component->at(i).w();
        }
        if (my < m_scroll_component->at(i).y()) {
            my = m_scroll_component->at(i).y() + m_scroll_component->at(i).h();
        }
    }
    mx += theme::scrollbox_margin();
    my += theme::scrollbox_margin();
    mx -= m_scroll_component->w();
    my -= m_scroll_component->h();
    if (mx < 0) {
        mx = 0;
    }
    if (my < 0) {
        my = 0;
    }
    m_max_scroll_x = mx;
    m_max_scroll_y = my;
    scroll_x(m_scroll_component->scroll_x());
    scroll_y(m_scroll_component->scroll_y());
    update_scrollbar_range();
}

void Scrollbox::sync_scrollbar() {
    if (m_inside_scroll_callback) return;
    m_inside_scroll_callback = true;
    scroll_x(m_horizontal_sb->value());
    scroll_y(m_vertical_sb->value());
    m_inside_scroll_callback = false;
}

void Scrollbox::update_scrollbar_range() {
    m_horizontal_sb->max(m_max_scroll_x);
    m_vertical_sb->max(m_max_scroll_y);
    scroll_visibility();
}

void Scrollbox::max_scroll_x(uint32_t value) {
    if (m_autoscroll) {
        compute_autoscroll();
        return;
    };
    m_max_scroll_x = value;
    update_scrollbar_range();
    scroll_x(m_scroll_component->scroll_x());
}

void Scrollbox::max_scroll_y(uint32_t value) {
    if (m_autoscroll) {
        compute_autoscroll();
        return;
    };
    m_max_scroll_y = value;
    update_scrollbar_range();
    scroll_y(m_scroll_component->scroll_y());   
}

uint32_t Scrollbox::max_scroll_x() {
    return m_max_scroll_x;
}

uint32_t Scrollbox::max_scroll_y() {
    return m_max_scroll_y;
}

void Scrollbox::scroll_visibility() {
    if (!m_vertical_sb) return;
    if (!m_horizontal_sb) return;
    m_vertical_sb->visible(m_vertical_sb->max() > 0);
    m_horizontal_sb->visible(m_horizontal_sb->max() > 0);
    positionate_scrollbars();
}

void Scrollbox::paint(sf::RenderTarget *render_target) {
    Drawing ar(Drawing::drawing_flat_box);
    ar.outline_color(m_outline_color);
    ar.color(m_fill_color);
    ar.size(abs_w(), abs_h());
    ar.position(abs_x(), abs_y());
    ar.margin(0);
    ar.draw(render_target);
}

} // namespace dfe_ui
