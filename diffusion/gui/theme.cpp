#include "simple-ui/theme.h"

namespace dfe_ui {
namespace theme {
namespace {
    // editor
    uint32_t editor_text_color_     = 0x000000FF;
    uint32_t editor_out_color_      = 0xFF0000FF;
    uint32_t editor_fill_color_     = 0xFFFFFFFF;
    uint32_t editor_cursor_color_   = 0x000000FF;
    uint32_t editor_selection_color_= 0x11111133;

    // button
    uint32_t button_text_color_     = 0x111111FF;
    uint32_t button_fill_color_     = 0x111111FF;
    uint32_t button_out_color_      = 0xFF0000FF;
    uint32_t button_highlighted_color_ = 0x222222FF;

    // label
    uint32_t label_text_color_      = 0x000000FF;
    
}

uint32_t editor_text_color() {
    return editor_text_color_;
}

uint32_t editor_outline_color() {
    return editor_out_color_;
}

uint32_t editor_fill_color() {
    return editor_fill_color_;
}

uint32_t editor_cursor_color() {
    return editor_cursor_color_;
}

uint32_t editor_selection_color() {
    return editor_selection_color_;
}

uint32_t button_text_color() {
    return button_text_color_;
}

uint32_t label_text_color() {
    return label_text_color_;
}

uint32_t button_fill_color() {
    return button_fill_color_;
}

uint32_t button_outline_color() {
    return button_out_color_;
}

void editor_text_color(uint32_t value) {
    editor_text_color_ = value;
}

void button_color(uint32_t text_color, uint32_t fill_color, uint32_t outline_color, uint32_t highlighted_color) {
    button_text_color_ = text_color;
    button_fill_color_ = fill_color;
    button_out_color_ = outline_color;
    button_highlighted_color_ = highlighted_color;
}

uint32_t button_highlighted_color() {
    return button_highlighted_color_;
}

void label_text_color(uint32_t value) {
    label_text_color_ = value;
}

    
} // namespace theme
} // namespace dfe_ui
