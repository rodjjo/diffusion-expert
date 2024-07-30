
#include <algorithm>
#include <string>
#include <map>
#include <stdexcept>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>

#include "simple-ui/component.h"
#include "simple-ui/win.h"

namespace dfe_ui {

namespace {
    bool global_damaged_ = true;
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

Component::Component(Window *window) : window_(window), items_(this) {
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
        if (child->parent_) {
            child->parent_->fire_child_count_changed();
        }
    }
    this->items_.add(child);
    child->parent_ = this;
    child->parent_changed();
    child->damaged(true);
    fire_child_count_changed();
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
        return enabled() && parent()->enabled();
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

void Component::float_on(Component *parent) {
    if (window_ != NULL && window_ != this) {
        floatting_parent_ = parent;
        window_->add_floating_commponent(this);
    }
}

void Component::float_off() {
    if (window_ != NULL && window_ != this) {
        window_->remove_floating_commponent(this);
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

    if (this->scale_ == 0) {
        return NULL;
    }

    int this_x = this->x();
    int this_y = this->y();

    float this_scale = this->scale();
    if (this_scale == 0) {
        return NULL;
    }

    if (floatting_parent_) {
        int px = 0;
        int py = 0;
        auto p = floatting_parent_;
        while (p) {
            px += p->x() - p->scroll_x_;
            py += p->y() - p->scroll_y_;
            p = p->parent();
        }
        this_scale *= floatting_parent_->abs_scale();
        if (this_scale == 0) {
            return NULL;
        }
        this_x += px;
        this_y += py;
    }
    
    x /= this_scale;
    y /= this_scale;

    int sx = 0;
    int sy = 0;
    if (parent_) {
        sx = parent_->scroll_x_;
        sy = parent_->scroll_y_;
    }  else if (window_ && window_ != this) {
        sx = window_->scroll_x_;
        sy = window_->scroll_y_;
    }

    int current_x = this_x - sx;
    int current_y = this_y - sy;
    int current_x2 = current_x + this->w();
    int current_y2 = current_y + this->h();
    if (x < current_x || x > current_x2 ||
        y < current_y || y > current_y2) {
        return NULL;
    }

    if (clickable()) {
        result = this;
    }

    x -= current_x;
    y -= current_y;
    int saved_x = x;
    int saved_y = y;

    Component *sub;
    for (size_t i = 0; i < items().size(); i++) {
        sub = items().at(i).find_top_clickable(x, y);
        if (sub) {
            result = sub;
            break;
        }
        x = saved_x;
        y = saved_y;
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
    int vx = dx + x_ * scale;
    auto p = parent();
    while (p) {
        dx = p->status() == component_status_dragging ? p->drag_x_ : 0;
        vx +=  dx + p->x_ * scale;         
        p = p->parent();
    }
    p = window_ != this ? window_ : NULL;
    if (p) {
        vx += p->x_ * scale;         
    }
    vx -= abs_scrollx();
    return vx + dx;
}

int Component::abs_y() {
    int dy = status() == component_status_dragging ? drag_y_ : 0;
    float scale = abs_scale();
    int vy = dy + y_ * scale;
    auto p = parent();
    while (p) {
        dy = p->status() == component_status_dragging ? p->drag_y_ : 0;
        vy += dy + p->y_ * scale;         
        p = p->parent();
    }
    p = window_ != this ? window_ : NULL;
    if (p) {
        vy += p->y_ * scale;         
    }
    vy -= abs_scrolly();
    return vy + dy;
}

int Component::abs_scrollx() {
    int sx = 0;
    auto p = parent_;
    if (!p) {
        p = floatting_parent_;
    }
    while (p) {
        sx += p->scroll_x_;
        if (!p->parent_) {
            p = p->floatting_parent_;
        } else {
            p = p->parent_;
        }
    }
    p = window_ != NULL && window_ != this && window_ != p ? window_ : NULL;
    while (p) {
        sx += p->scroll_x_;
        p = p->parent_;
    }
    return sx * abs_scale();
}

int Component::abs_scrolly() {
    int sy = 0;
    auto p = parent_;
    if (!p) {
        p = floatting_parent_;
    }
    while (p) {
        sy += p->scroll_y_;
        if (!p->parent_) {
            p = p->floatting_parent_;
        } else {
            p = p->parent_;
        }
    }
    p = window_ != NULL && window_ != this && window_ != p ? window_ : NULL;
    while (p) {
        sy += p->scroll_y_;
        p = p->parent_;
    }
    return sy * abs_scale();

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
        return scale_ * parent()->abs_scale();
    }
    if (floatting_parent_) {
        return scale_ * floatting_parent_->abs_scale();
    }
    if (window_ && window_ != this) {
        return scale_ * window_->abs_scale();
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

component_cursor_t Component::cursor() {
    return cursor_arrow;
}

Component *Component::parent() {
    if (parent_) {
        return parent_;
    }
    return floatting_parent_;
}

bool Component::is_floatting() {
    if (window_) {
        return window_->is_floatting_component(this);
    }
    return false;
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

void Component::fire_child_count_changed() {
    handle_child_count_changed();
}



}  // namespace dfe 