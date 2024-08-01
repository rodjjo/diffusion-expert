#include "drawings.h"
#include "simple-ui/text_box.h"
#include "simple-ui/theme.h"


namespace dfe_ui
{

TextBox::TextBox(Window * window, int x, int y, int w, int h) : Edit(window, x, y, w, h) {
    text_color(dfe_ui::theme::editor_text_color());
    cursor_color(dfe_ui::theme::editor_cursor_color());
    selection_color(dfe_ui::theme::editor_selection_color());
    outline_color(dfe_ui::theme::editor_outline_color());
    color(dfe_ui::theme::editor_fill_color());
}

TextBox::~TextBox() {

}

void TextBox::color(uint32_t value) {
    m_color = value;
}

uint32_t TextBox::color() {
    return m_color;
}

void TextBox::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t TextBox::outline_color() {
    return m_outline_color;
}

void TextBox::paint(sf::RenderTarget *render_target) {
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);
    dw.color(m_color);
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);
    Edit::paint(render_target);
};

} // namespace dfe_ui
