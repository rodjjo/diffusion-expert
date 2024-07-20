#include <algorithm>
#include <string>
#include <stdexcept>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "simple-ui/component.h"

namespace dfe_ui {

namespace {
    bool global_damaged_ = true;
}

PaintingContext::PaintingContext(int target_h, Component *component) {
    int sz[4];
    glGetIntegerv(GL_SCISSOR_BOX, &sz[0]);
    view_x_ = sz[0];
    view_y_ = sz[1];
    view_w_ = sz[2];
    view_h_ = sz[3];

    int nx = component->abs_x();
    int ny = target_h - (component->abs_y() + component->h());
    int nw = component->w();
    int nh = component->h();

    if (!glIsEnabled(GL_SCISSOR_TEST)) {
        disable_scissor_ = true;
        glEnable(GL_SCISSOR_TEST);
    } else {
        if (nx < sz[0]) {
            nw = nw - (sz[0] - nx);
            nx = sz[0];
            if (nw < 0) {
                nw = 0;
            }
        }
        if (nx > sz[0] + sz[2]) {
            nw = 0;
        }
        if (nw > 0 && nx + nw > sz[0] + sz[2]) {
            nw = sz[0] + sz[2] - nx;
        }
        if (ny < sz[1]) {
            nh = nh - (sz[1] - ny);
            ny = sz[1];
            if (nh < 0) {
                nh = 0;
            }
        }
        if (ny + nh > sz[1] + sz[3]) {
            nh = nh - ((ny + nh) - (sz[1] + sz[3]));
            if (nh < 0) {
                nh = 0;
            }
        }
    }
    
    glScissor(nx, ny, nw, nh);
}

PaintingContext::~PaintingContext() {
    glScissor(view_x_, view_y_, view_w_, view_h_);
    if (disable_scissor_) {
        glDisable(GL_SCISSOR_TEST);
    }
}

ComponentList::ComponentList(Component *parent) : parent_(parent) {

}

ComponentList::~ComponentList() {

}

Component & ComponentList::operator[] (size_t index) {
    return at(index);
}

size_t ComponentList::size() {
    return items_.size();
}

Component & ComponentList::at(size_t index) {
    return *items_[index].get();
}

bool ComponentList::empty() {
    return items_.empty();
}

void ComponentList::add(std::shared_ptr<Component> component) {
    items_.push_back(component);
    sort();
}

void ComponentList::remove(Component *element) {
    for (auto iterator = items_.begin(); iterator != items_.end(); iterator++) {
        if (iterator->get() == element) {
            element->parent_ = NULL;
            items_.erase(iterator);
            sort();
            return;
        }
    }
}

void ComponentList::sort() {
    std::sort(items_.begin(), items_.end(), [](const std::shared_ptr<Component>& a, const std::shared_ptr<Component>& b) {return a->zorder() > b->zorder();});
}

Component::Component() : items_(this) {
}

Component::~Component() {
}

void Component::paint(void *render_window) {
    damaged_ = false;
}

std::shared_ptr<Component> Component::share() {
    return shared_from_this();
}

void Component::add(std::shared_ptr<Component> child) {
    if (child->parent_) {
        child->parent_->items_.remove(child.get());
    }
    this->items_.add(child);
    child->parent_ = this;
}

int Component::zorder() const {
    return z_order_;
}

void Component::zorder(int value) {
    if (value == z_order_) return;
    damaged_ = true;
    z_order_ = value;
    if (parent_) {
        parent_->items_.sort();
    }
}

bool Component::damaged() {
    return damaged_;
}

void Component::damaged(bool value) {
    if (value) {
        global_damaged_ = true;
    }
    damaged_ = value;
}

bool Component::visible() {
    return visible_;
}

void Component::visible(bool value) {
    visible_ = value;
}

bool Component::drag_enabled() {
    return drag_enabled_;
}

void Component::drag_enabled(bool value) {
    drag_enabled_ = value;
}

bool Component::drop_enabled() {
    return drop_enabled_;
}

void Component::drop_enabled(bool value) {
    drop_enabled_ = value;
}

int Component::x() {
    return x_;
}

int Component::y() {
    return y_;
}

int Component::w() {
    return w_;
}

int Component::h() {
    return h_;
}

bool Component::enabled() {
    return enabled_;
}

void Component::enabled(bool value) {
    enabled_ = value;
}

void Component::x(int value) {
    if (x_ == value) return;
    x_ = value;
    damaged(true);
    fire_parent_resized();
}

void Component::y(int value) {
    if (y_ == value) return;
    y_ = value;
    damaged(true);
    fire_parent_resized();
}

void Component::w(int value) {
    if (w_ == value) return;
    w_ = value;
    damaged(true);
    fire_parent_resized();
}

void Component::h(int value) {
    if (h_ == value) return;
    h_ = value;
    damaged(true);
    fire_parent_resized();
}

void Component::size(int w, int h) {
    if (h_ == h && w_ == w) return;
    w_ = w;
    h_ = h;
    damaged(true);
    fire_parent_resized();
}

void Component::coordinates(int x, int y, int w, int h) {
    if (h_ == h && w_ == w && x_ == x && y_ == y) return;
    y_ = y;
    x_ = x;
    w_ = w;
    h_ = h;
    damaged(true);
    fire_parent_resized();
}

bool Component::clickable() {
    return false;
}

bool Component::editable() {
    return false;
}

bool Component::focusable() {
    return false;
}

void Component::handle_parent_resized() {
}

void Component::handle_textentered(uint32_t unicode) {
}

void Component::handle_mouse_left_pressed(int x, int y) {
}

void Component::handle_mouse_middle_pressed(int x, int y) {
}

void Component::handle_mouse_right_pressed(int x, int y) {
}

void Component::handle_mouse_left_released(int x, int y) {
}

void Component::handle_mouse_middle_released(int x, int y) {
}

void Component::handle_mouse_right_released(int x, int y) {
}

void Component::handle_mouse_moved(int x, int y) {
}

void Component::handle_keypressed(int key) {

}

void Component::handle_mouse_wheel(int8_t direction, int x, int y) {
}

void Component::handle_focus_lost() {

}

void Component::handle_focus_got() {

}

void Component::fire_parent_resized() {
    handle_parent_resized();
    for (auto i = 0; i < items_.size(); i++) {
        items_[i].fire_parent_resized();
    }
}

Component *Component::find_top_clickable(int &x, int &y) {
    if (!visible() || !enabled()) {
        return NULL;
    }

    Component *result = NULL, *next = NULL;

    if (clickable()) {
        result = this;
    }

    x -= this->x();
    y -= this->y();

    for (size_t i = 0; i < items_.size() && next == NULL; i++) {
        if (!items_[i].enabled()) continue;   
        if (!items_[i].visible()) continue;   
        if (items_[i].x() > x) continue;   
        if (items_[i].y() > y) continue;
        if (items_[i].x() + items_[i].w() < x) continue;
        if (items_[i].y() + items_[i].h() < y) continue;
        next = items_[i].find_top_clickable(x, y);
    }

    if (next) {
        result = next;
    }

    return result;
}

void Component::paint_children(void *render_window) {
    if (!visible()) {
        return;
    }
    PaintingContext context(static_cast<sf::RenderWindow *>(render_window)->getSize().y, this);
    paint(render_window);
    for (size_t i = 0; i < items_.size(); i++) {
        items_[i].paint_children(render_window);
    }
}

int Component::abs_x() {
    if (parent_) {
        return x_ + parent_->x_;
    }
    return x_;
}

int Component::abs_y() {
    if (parent_) {
        return y_ + parent_->y_;
    }
    return y_;
}



}  // namespace dfe 