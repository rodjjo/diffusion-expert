
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
    bool m_global_damaged = true;
}

// Stackable Scissor context
class ScissorContext {
    void init(int target_h, int nx, int ny, int nw, int nh) {
        int sz[4];
        glGetIntegerv(GL_SCISSOR_BOX, &sz[0]);
        m_view_x = sz[0];
        m_view_y = sz[1];
        m_view_w = sz[2];
        m_view_h = sz[3];

        if (!glIsEnabled(GL_SCISSOR_TEST)) {
            m_disable_scissor = true;
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

  public:
    ScissorContext(int target_h, int nx, int ny, int nw, int nh) {
        init(
            target_h, 
            nx, 
            target_h - (ny + nh),
            nw,
            nh
        );
    }
    ScissorContext(int target_h, Component *component) {
        init(
            target_h, 
            component->abs_x(), 
            target_h - (component->abs_y() + component->abs_h()),
            component->abs_w(),
            component->abs_h()
        );
    }

    ~ScissorContext() {
        glScissor(m_view_x, m_view_y, m_view_w, m_view_h);
        if (m_disable_scissor) {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    bool visible() {
        if (m_view_w < 1) return false;
        if (m_view_h < 1) return false;
        return true;
    }
  private:
    bool m_disable_scissor = false;
    float m_view_x = 0;
    float m_view_y = 0;
    float m_view_w = 0;
    float m_view_h = 0;
};


Component & Component::operator[] (size_t index) {
    return at(index);
}

size_t Component::component_count() {
    return m_items.size();
}



Component & Component::at(size_t index) {
    return *m_items[index].get();
}

void Component::add_component(std::shared_ptr<Component> component) {
    m_items.push_back(component);
    sort_components();
}

void Component::remove_component(Component *element) {
    for (auto iterator = m_items.begin(); iterator != m_items.end(); iterator++) {
        if (iterator->get() == element) {
            element->m_parent = NULL;
            m_items.erase(iterator);
            sort_components();
            return;
        }
    }
}

void Component::sort_components() {
    std::sort(m_items.begin(), m_items.end(), [](const std::shared_ptr<Component>& a, const std::shared_ptr<Component>& b) {return a->zorder() > b->zorder();});
}

Component::Component(Window *window) : m_window(window) {
}

Component::~Component() {
}

void Component::paint(sf::RenderTarget *render_target) {
    
}

std::shared_ptr<Component> Component::share() {
    return shared_from_this();
}

void Component::add(std::shared_ptr<Component> child) {
    if (child->m_parent) {
        child->m_parent->remove_component(child.get());
        if (child->m_parent) {
            child->m_parent->fire_child_count_changed();
        }
    }
    this->add_component(child);
    child->m_parent = this;
    child->parent_changed();
    fire_child_count_changed();
}

int Component::zorder() const {
    return m_z_order;
}

void Component::zorder(int value) {
    if (value == m_z_order) return;
    m_z_order = value;
    if (m_parent) {
        m_parent->sort_components();
    }
}

bool Component::visible() {
    return m_visible;
}

void Component::visible(bool value) {
    m_visible = value;
}

bool Component::drag_enabled() {
    return false;
}


bool Component::drop_enabled() {
    return false;
}

int Component::x() {
    return m_x;
}

int Component::y() {
    return m_y;
}

int Component::w() {
    return m_w;
}

int Component::h() {
    return m_h;
}

bool Component::abs_enabled() {
    if (m_parent) {
        return enabled() && parent()->enabled();
    }
    return enabled();
}

bool Component::enabled() {
    return m_enabled;
}

void Component::enabled(bool value) {
    m_enabled = value;
}

void Component::x(int value) {
    if (m_x == value) return;
    m_x = value;
    fire_parent_resized();
}

void Component::y(int value) {
    if (m_y == value) return;
    m_y = value;
    fire_parent_resized();
}

void Component::w(int value) {
    if (m_w == value) return;
    m_w = value;
    
    fire_parent_resized();
}

void Component::h(int value) {
    if (m_h == value) return;
    m_h = value;
    
    fire_parent_resized();
}

void Component::size(int w, int h) {
    if (m_h == h && m_w == w) return;
    m_w = w;
    m_h = h;
    
    fire_parent_resized();
}

void Component::coordinates(int x, int y, int w, int h) {
    if (m_h == h && m_w == w && m_x == x && m_y == y) return;
    m_y = y;
    m_x = x;
    m_w = w;
    m_h = h;
    
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
    return m_tag;
}

void Component::tag(size_t value) {
    m_tag = value;
}


void Component::drop_begin(Component *source) {
    if (accept_drop(source)) {
        drop_begin();
    }
    for (size_t i = 0; i < m_items.size(); i++) {
        m_items[i]->drop_begin(source);
    }
}

void Component::drop_end(Component *source) {
    if (accept_drop(source)) {
        drop_end();
    }
    for (size_t i = 0; i < m_items.size(); i++) {
        m_items[i]->drop_end(source);
    }
}

void Component::float_on(Component *parent) {
    if (m_window != NULL && m_window != this) {
        m_floatting_parent = parent;
        m_window->add_floating_commponent(this);
    }
}

void Component::float_off() {
    if (m_window != NULL && m_window != this) {
        m_window->remove_floating_commponent(this);
    }
}

void Component::fire_parent_resized() {
    handle_parent_resized();
    for (auto i = 0; i < m_items.size(); i++) {
        m_items[i]->fire_parent_resized();
    }
}

Component *Component::find_top_clickable(int &x, int &y) {
    if (!visible() || !enabled()) {
        return NULL;
    }
    Component *result = NULL, *next = NULL;

    if (this->m_scale == 0) {
        return NULL;
    }

    int this_x = this->x();
    int this_y = this->y();

    float this_scale = this->scale();
    if (this_scale == 0) {
        return NULL;
    }

    if (m_floatting_parent) {
        int px = 0;
        int py = 0;
        auto p = m_floatting_parent;
        while (p) {
            px += p->x() - p->m_scroll_x;
            py += p->y() - p->m_scroll_y;
            p = p->parent();
        }
        this_scale *= m_floatting_parent->abs_scale();
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
    if (m_parent) {
        sx = m_parent->m_scroll_x;
        sy = m_parent->m_scroll_y;
    }  else if (m_window && m_window != this) {
        sx = m_window->m_scroll_x;
        sy = m_window->m_scroll_y;
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
    for (size_t i = 0; i < m_items.size(); i++) {
        sub = m_items[i]->find_top_clickable(x, y);
        if (sub) {
            result = sub;
            break;
        }
        x = saved_x;
        y = saved_y;
    }

    return result;
}

void Component::paint_children(sf::RenderTarget *render_target, bool check_status) {
    if (!visible() || (status() == component_status_dragging && check_status)) {
        return;
    }

    ScissorContext context(render_target->getSize().y, this);
    if (context.visible()) {
        paint(render_target);
        for (size_t i = 0; i < m_items.size(); i++) {
            m_items[i]->paint_children(render_target);
        }
    }
}

void Component::paint_constraint(int x, int y, int w, int h, int render_y, std::function<void()> cb) {
    ScissorContext context(render_y, x, y, w, h);
    if (context.visible()) {
        cb();
    }
}

int Component::abs_x() {
    int dx = status() == component_status_dragging ? m_drag_x : 0;
    float scale = abs_scale();
    int vx = dx + m_x * scale;
    auto p = parent();
    while (p) {
        dx = p->status() == component_status_dragging ? p->m_drag_x : 0;
        vx +=  dx + p->m_x * scale;         
        p = p->parent();
    }
    p = m_window != this ? m_window : NULL;
    if (p) {
        vx += p->m_x * scale;         
    }
    vx -= abs_scrollx();
    return vx + dx;
}

int Component::abs_y() {
    int dy = status() == component_status_dragging ? m_drag_y : 0;
    float scale = abs_scale();
    int vy = dy + m_y * scale;
    auto p = parent();
    while (p) {
        dy = p->status() == component_status_dragging ? p->m_drag_y : 0;
        vy += dy + p->m_y * scale;         
        p = p->parent();
    }
    p = m_window != this ? m_window : NULL;
    if (p) {
        vy += p->m_y * scale;         
    }
    vy -= abs_scrolly();
    return vy + dy;
}

int Component::abs_scrollx() {
    int sx = 0;
    auto p = m_parent;
    if (!p) {
        p = m_floatting_parent;
    }
    while (p) {
        sx += p->m_scroll_x;
        if (!p->m_parent) {
            p = p->m_floatting_parent;
        } else {
            p = p->m_parent;
        }
    }
    p = m_window != NULL && m_window != this && m_window != p ? m_window : NULL;
    while (p) {
        sx += p->m_scroll_x;
        p = p->m_parent;
    }
    return sx * abs_scale();
}

int Component::abs_scrolly() {
    int sy = 0;
    auto p = m_parent;
    if (!p) {
        p = m_floatting_parent;
    }
    while (p) {
        sy += p->m_scroll_y;
        if (!p->m_parent) {
            p = p->m_floatting_parent;
        } else {
            p = p->m_parent;
        }
    }
    p = m_window != NULL && m_window != this && m_window != p ? m_window : NULL;
    while (p) {
        sy += p->m_scroll_y;
        p = p->m_parent;
    }
    return sy * abs_scale();
}


int Component::scroll_x() {
    return m_scroll_x;
}

int Component::scroll_y() {
    return m_scroll_y;
}

float Component::scale() {
    return m_scale;
}

float Component::abs_scale() {
    if (m_parent) {
        return m_scale * parent()->abs_scale();
    }
    if (m_floatting_parent) {
        return m_scale * m_floatting_parent->abs_scale();
    }
    if (m_window && m_window != this) {
        return m_scale * m_window->abs_scale();
    }
    return m_scale;
}

int Component::abs_w() {
    return m_w * abs_scale();
}

int Component::abs_h() {
    return m_h * abs_scale();
}

void Component::scroll_x(int value) {
    m_scroll_x = value;
}

void Component::scroll_y(int value) {
    m_scroll_y = value;
}

void Component::set_drag_coord(int x, int y) {
    m_drag_x = x;
    m_drag_y = y;
}

void Component::scale(float value) {
    m_scale = value;
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
    if (m_parent) {
        return m_parent;
    }
    return m_floatting_parent;
}

bool Component::is_floatting() {
    if (m_window) {
        return m_window->is_floatting_component(this);
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

Window *Component::window() {
    return m_window;
}


}  // namespace dfe 