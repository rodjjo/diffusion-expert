#include <stdio.h>
#include "simple-ui/theme.h"
#include "simple-ui/scrollbox.h"
#include "drawings.h"

namespace dfe_ui {


Scrollbox::Scrollbox(Window * window, int x, int y, int w, int h) : Component(window) {
    scroll_component_.reset(new Component(window));
    scroll_component_->coordinates(0, 0, w, h);
    this->coordinates(x, y, w, h);
    vertical_sb_.reset(new Scrollbar(window, w, 0, w - theme::scrollbox_scrollbar_size(), h, true));
    horizontal_sb_.reset(new Scrollbar(window, 0, h - theme::scrollbox_scrollbar_size(), w - theme::scrollbox_scrollbar_size(), theme::scrollbox_scrollbar_size(), false));
    horizontal_sb_->onchange([this](Component *) {
        sync_scrollbar();
    });
    vertical_sb_->onchange([this](Component *) {
        sync_scrollbar();
    });
    vertical_sb_->visible(false);
    horizontal_sb_->visible(false);
    Component::add(scroll_component_);
    Component::add(vertical_sb_);
    Component::add(horizontal_sb_);
    handle_parent_resized();
    fill_color(theme::scrollbox_color());
    outline_color(theme::scrollbox_outline_color());
}

Scrollbox::~Scrollbox() {
}

uint32_t Scrollbox::fill_color() {
    return fill_color_;
}

void Scrollbox::fill_color(uint32_t value) {
    fill_color_ = value;
}

void Scrollbox::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t Scrollbox::outline_color() {
    return outline_color_;
}

bool Scrollbox::autoscroll() {
    return autoscroll_;
}

void Scrollbox::autoscroll(bool value) {
    autoscroll_ = value;
}

void Scrollbox::scroll_x(int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > max_scroll_x_) {
        value = max_scroll_x_;
    }
    scroll_component_->scroll_x(value);
}

void Scrollbox::scroll_y(int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > max_scroll_y_) {
        value = max_scroll_y_;
    }
    scroll_component_->scroll_y(value);
}

void Scrollbox::add(std::shared_ptr<Component> child) {
    scroll_component_->add(child);
    compute_autoscroll();
};

ComponentList & Scrollbox::scroll_items() {
    return scroll_component_->items();
};


void Scrollbox::handle_parent_resized() {
    if (!vertical_sb_) return;
    if (!horizontal_sb_) return;
    horizontal_sb_->coordinates(0, h() - theme::scrollbox_scrollbar_size(), w() - theme::scrollbox_scrollbar_size(), theme::scrollbox_scrollbar_size());
    vertical_sb_->coordinates(w() - theme::scrollbox_scrollbar_size(), 0, theme::scrollbox_scrollbar_size(), h() - theme::scrollbox_scrollbar_size());
    compute_autoscroll();
    positionate_scrollbars();
};

void Scrollbox::positionate_scrollbars() {
    int w = this->w();
    int h = this->h();
    if (horizontal_sb_->visible()) {
        h = horizontal_sb_->y();
    }
    if (vertical_sb_->visible()) {
        w = vertical_sb_->x();
    }
    scroll_component_->coordinates(0, 0, w, h);
}

void Scrollbox::handle_child_count_changed() {
    compute_autoscroll();
};

void Scrollbox::compute_autoscroll() {
    if (!autoscroll_) return;
    if (!scroll_component_) return;
    int mx = 0;
    int my = 0;
    for (size_t i = 0; i < scroll_component_->items().size(); i++) {
        if (mx < scroll_component_->items().at(i).x()) {
            mx = scroll_component_->items().at(i).x() + scroll_component_->items().at(i).w();
        }
        if (my < scroll_component_->items().at(i).y()) {
            my = scroll_component_->items().at(i).y() + scroll_component_->items().at(i).h();
        }
    }
    mx += theme::scrollbox_margin();
    my += theme::scrollbox_margin();
    mx -= scroll_component_->w();
    my -= scroll_component_->h();
    if (mx < 0) {
        mx = 0;
    }
    if (my < 0) {
        my = 0;
    }
    max_scroll_x_ = mx;
    max_scroll_y_ = my;
    scroll_x(scroll_component_->scroll_x());
    scroll_y(scroll_component_->scroll_y());
    update_scrollbar_range();
}

void Scrollbox::sync_scrollbar() {
    if (inside_scroll_callback_) return;
    inside_scroll_callback_ = true;
    scroll_x(horizontal_sb_->value());
    scroll_y(vertical_sb_->value());
    inside_scroll_callback_ = false;
}

void Scrollbox::update_scrollbar_range() {
    horizontal_sb_->max(max_scroll_x_);
    vertical_sb_->max(max_scroll_y_);
    scroll_visibility();
}

void Scrollbox::max_scroll_x(uint32_t value) {
    if (autoscroll_) {
        compute_autoscroll();
        return;
    };
    max_scroll_x_ = value;
    update_scrollbar_range();
    scroll_x(scroll_component_->scroll_x());
}

void Scrollbox::max_scroll_y(uint32_t value) {
    if (autoscroll_) {
        compute_autoscroll();
        return;
    };
    max_scroll_y_ = value;
    update_scrollbar_range();
    scroll_y(scroll_component_->scroll_y());   
}

uint32_t Scrollbox::max_scroll_x() {
    return max_scroll_x_;
}

uint32_t Scrollbox::max_scroll_y() {
    return max_scroll_y_;
}

void Scrollbox::scroll_visibility() {
    if (!vertical_sb_) return;
    if (!horizontal_sb_) return;
    vertical_sb_->visible(vertical_sb_->max() > 0);
    horizontal_sb_->visible(horizontal_sb_->max() > 0);
    positionate_scrollbars();
}

void Scrollbox::paint(void *render_window) {
    Drawing ar(Drawing::drawing_flat_box);
    ar.outline_color(outline_color_);
    ar.color(fill_color_);
    ar.size(abs_w(), abs_h());
    ar.position(abs_x(), abs_y());
    ar.margin(0);
    ar.draw(render_window);
}

} // namespace dfe_ui
