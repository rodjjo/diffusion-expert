#include "simple-ui/modal.h"
#include "simple-ui/win.h"
#include "simple-ui/theme.h"
#include "drawings.h"


namespace dfe_ui {

Modal::Modal(Window *window) : Component(window) {
    fill_color(theme::modal_fill_color());
}

Modal::~Modal() {
}

void Modal::fill_color(uint32_t value) {
    m_fill_color = value;
}

uint32_t Modal::fill_color() {
    return m_fill_color;
}

void Modal::paint(sf::RenderTarget *render_target) {
    Drawing dw(Drawing::drawing_flat_box);
    dw.position(abs_x(), abs_y());
    dw.size(abs_w(), abs_h());
    dw.color(m_fill_color);
    dw.outline_color(m_fill_color);
    dw.draw(render_target);
}

void Modal::handle_parent_resized() {
    coordinates(0, 0, window()->w(), window()->h());
}

void Modal::float_on(Component *parent) {
    handle_parent_resized();
    Component::float_on(parent);
}

bool Modal::clickable() {
    return true;
}

}
