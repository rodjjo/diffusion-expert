#include "drawings.h"
#include "simple-ui/text_box.h"
#include "simple-ui/theme.h"


namespace dfe_ui
{

TextBox::TextBox(int x, int y, int w, int h) : Edit(x, y, w, h) {
    text_color(dfe_ui::theme::editor_text_color());
    cursor_color(dfe_ui::theme::editor_cursor_color());
    selection_color(dfe_ui::theme::editor_selection_color());
    outline_color(dfe_ui::theme::editor_outline_color());
    color(dfe_ui::theme::editor_fill_color());
}

TextBox::~TextBox() {

}

void TextBox::color(uint32_t value) {
    color_ = value;
}

uint32_t TextBox::color() {
    return color_;
}

void TextBox::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t TextBox::outline_color() {
    return outline_color_;
}

void TextBox::paint(void *render_window) {
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(outline_color_);
    dw.color(color_);
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_window);
    Edit::paint(render_window);
};

} // namespace dfe_ui
