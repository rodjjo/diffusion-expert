#include <algorithm>
#include <string>
#include <stdexcept>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>

#include "simple-ui/component.h"

namespace dfe_ui {

namespace {
    bool global_damaged_ = true;
    int64_t time_counter = 0;
    sf::Clock clock;
}

ScissorContext::ScissorContext(int target_h, Component *component) {
    int sz[4];
    glGetIntegerv(GL_SCISSOR_BOX, &sz[0]);
    view_x_ = sz[0];
    view_y_ = sz[1];
    view_w_ = sz[2];
    view_h_ = sz[3];

    int nx = component->abs_x();
    int ny = target_h - (component->abs_y() + component->abs_h());
    int nw = component->abs_w();
    int nh = component->abs_h();

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

ScissorContext::~ScissorContext() {
    glScissor(view_x_, view_y_, view_w_, view_h_);
    if (disable_scissor_) {
        glDisable(GL_SCISSOR_TEST);
    }
}

bool ScissorContext::visible() {
    if (view_w_ < 1) return false;
    if (view_h_ < 1) return false;
    return true;
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
        child->parent_->damaged(true);
        child->parent_->items_.remove(child.get());
    }
    this->items_.add(child);
    child->parent_ = this;
    child->parent_changed();
    child->damaged(true);
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
    return false;
}


bool Component::drop_enabled() {
    return false;
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

bool Component::abs_enabled() {
    if (parent_) {
        return enabled() && parent_->enabled();
    }
    return enabled();
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

void Component::parent_changed() {
}

void Component::handle_parent_resized() {
}

void Component::handle_textentered(wchar_t unicode) {
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

void Component::drag_begin() {
}

void Component::drag_end() {
}

void Component::drop_begin() {
}

void Component::drop_end() {
}

void Component::mouse_enter() {
}

void Component::mouse_exit() {
}

void Component::complete_drop(Component *comp) {
}

size_t Component::tag() {
    return tag_;
}

void Component::tag(size_t value) {
    tag_ = value;
}


void Component::drop_begin(Component *source) {
    if (accept_drop(source)) {
        drop_begin();
    }
    for (size_t i = 0; i < items_.size(); i++) {
        items_[i].drop_begin(source);
    }
}

void Component::drop_end(Component *source) {
    if (accept_drop(source)) {
        drop_end();
    }
    for (size_t i = 0; i < items_.size(); i++) {
        items_[i].drop_end(source);
    }
}



ComponentList & Component::items() {
    return items_;
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

    float abs_scale = this->abs_scale();
    //if (abs_scale != 0) {
        // abs_scale = 1.0 / abs_scale;
    //}

    x -= this->x() * abs_scale;
    y -= this->y() * abs_scale;

    for (size_t i = 0; i < items_.size() && next == NULL; i++) {
        if (!items_[i].enabled()) continue;   
        if (!items_[i].visible()) continue;   
        if (items_[i].x() * abs_scale > x) continue;   
        if (items_[i].y() * abs_scale > y) continue;
        if ((items_[i].x() + items_[i].w()) * abs_scale < x) continue;
        if ((items_[i].y() + items_[i].h()) * abs_scale < y) continue;
        next = items_[i].find_top_clickable(x, y);
    }

    if (next) {
        result = next;
    }

    return result;
}

void Component::paint_children(void *render_window, bool check_status) {
    if (!visible() || (status() == component_status_dragging && check_status)) {
        return;
    }

    ScissorContext context(static_cast<sf::RenderWindow *>(render_window)->getSize().y, this);
    if (context.visible()) {
        paint(render_window);
        for (size_t i = 0; i < items_.size(); i++) {
            items_[i].paint_children(render_window);
        }
    }
}

int Component::abs_x() {
    int dx = status() == component_status_dragging ? drag_x_ : 0;
    float scale = abs_scale();
    if (parent_) {
        return (x_ * scale)  + parent_->abs_x() - (scroll_x_ * scale) + dx;
    }
    return (x_ * scale) - (scroll_x_ * scale) + dx;
}

int Component::abs_y() {
    int dy = status() == component_status_dragging ? drag_y_ : 0;
    float scale = abs_scale();
    if (parent_) {
        return (y_ * scale)  + parent_->abs_y() - (scroll_y_ * scale) + dy;
    }
    return (y_ * scale) - (scroll_y_ * scale) + dy;
}

int Component::scroll_x() {
    return scroll_x_;
}

int Component::scroll_y() {
    return scroll_y_;
}

float Component::scale() {
    return scale_;
}

float Component::abs_scale() {
    if (parent_) {
        return scale_ * parent_->abs_scale();
    }
    return scale_;
}

int Component::abs_w() {
    return w_ * abs_scale();
}

int Component::abs_h() {
    return h_ * abs_scale();
}

void Component::scroll_x(int value) {
    scroll_x_ = value;
}

void Component::scroll_y(int value) {
    scroll_y_ = value;
}

void Component::set_drag_coord(int x, int y) {
    drag_x_ = x;
    drag_y_ = y;
}

void Component::scale(float value) {
    scale_ = value;
}

bool Component::accept_drag(Component *comp) {
    return false;
}

bool Component::accept_drop(Component *comp) {
    return false;
}

component_status_t Component::status() {
    return component_status_normal;
}

Component *Component::parent() {
    return parent_;
}

void Component::restart_clock() {
    time_counter = clock.restart().asMicroseconds();
}

int64_t Component::microseconds() {
    return clock.getElapsedTime().asMicroseconds();
}

int32_t Component::miliseconds() {
    return clock.getElapsedTime().asMicroseconds();
}

int Component::compute_text_min_y(void *text_shape) {
  sf::Text & text = *static_cast<sf::Text *>(text_shape);
  // all the characters has a coordinate to render (x, y)
  // this function return the minimal y's coordinate of the text, so we can center it correctly.
  int minY = 0;
  unsigned int csz = text.getCharacterSize();
  bool b = (text.getStyle() & sf::Text::Bold) != 0;
  minY = (int)csz;
  for (unsigned char c = 33; c <= 128; ++c) {
    const sf::Glyph& glp = text.getFont().getGlyph(c, csz , b);
    if (glp.bounds.position.y + static_cast<int>(csz) < minY)
      minY = glp.bounds.position.y + csz;
  }
  minY = abs(minY);
  return minY;
}

}  // namespace dfe 