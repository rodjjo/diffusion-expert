#include "simple-ui/theme.h"

namespace dfe_ui {
namespace theme {
namespace {
    // editor
    uint32_t editor_text_color_             = 0x000000FF;
    uint32_t editor_out_color_              = 0xFF0000FF;
    uint32_t editor_fill_color_             = 0xFFFFFFFF;
    uint32_t editor_cursor_color_           = 0x000000FF;
    uint32_t editor_selection_color_        = 0x11111133;
    uint32_t editor_line_spacing_           = 1;
    uint32_t editor_margin_                 = 4;
    // button
    uint32_t button_text_color_             = 0x111111FF;
    uint32_t button_fill_color_             = 0x444444FF;
    uint32_t button_pressed_color_          = 0x555555FF;
    uint32_t button_out_color_              = 0xFF0000FF;
    uint32_t button_highlighted_color_      = 0x222222FF;

    // label
    uint32_t label_text_color_              = 0x000000FF;

    // icons
    uint8_t icon_margin_                    = 5;

    // scrollbar
    uint32_t scrollbar_color_               = 0xAAAAAAFF;
    uint32_t scrollbar_highlighted_color_   = 0xBBBBBBFF;

    // progressbar
    uint32_t progress_bar_color_            = 0xFFFFFFFF;
    uint32_t progress_bar_outline_color_    = 0x000000FF;
    uint32_t progress_bar_text_color_       = 0x888888FF;
    uint32_t progress_bar_progress_color_   = 0x1100DDFF;

    // listbox
    uint8_t listbox_itemargin_               = 3;
    uint32_t listbox_selected_color_         = 0x000000FF;
    uint32_t listbox_selected_text_color_    = 0xFFFFFFFF;

    // scrollbox
    uint32_t scrollbox_margin_               = 10;    
    uint32_t scrollbox_scrollbar_size_       = 20;
    uint32_t scrollbox_color_                = 0xBCBCBCFF;
    uint32_t scrollbox_outline_color_        = 0xCCCCCCFF;

    // modal
    uint32_t modal_fill_color_               = 0x000000CC;
    
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

uint32_t button_pressed_color() {
    return button_pressed_color_;
}

void editor_color(uint32_t text_color, uint32_t fill_color, uint32_t outline_color, 
    uint32_t cursor_color, uint32_t selection_color) {
    editor_text_color_ = text_color;
    editor_cursor_color_ = cursor_color;
    editor_fill_color_ = fill_color;
    editor_out_color_ = outline_color;
    editor_selection_color_ = selection_color;
}

void button_color(uint32_t text, uint32_t fill, uint32_t outline, uint32_t highlighted, uint32_t pressed){
    button_text_color_ = text;
    button_fill_color_ = fill;
    button_out_color_ = outline;
    button_highlighted_color_ = highlighted;
    button_pressed_color_ = pressed;
}

uint32_t button_highlighted_color() {
    return button_highlighted_color_;
}

void label_text_color(uint32_t value) {
    label_text_color_ = value;
}


uint8_t icon_margin() {
    return icon_margin_;
}

void icon_margin(uint8_t value) {
    icon_margin_ = value;
}

uint32_t scroolbar_color() {
    return scrollbar_color_;
}

uint32_t scroolbar_highlighted_color() {
    return scrollbar_highlighted_color_;
}

uint32_t progress_bar_color() {
    return progress_bar_color_;
}

uint32_t progress_bar_outline_color() {
    return progress_bar_outline_color_;
}

uint32_t progress_bar_text_color() {
    return progress_bar_text_color_;
}

uint32_t progress_bar_progress_color() {
    return progress_bar_progress_color_;
}

uint8_t listbox_item_margin() {
    return listbox_itemargin_;
}

uint32_t listbox_selected_color() {
    return listbox_selected_color_;
}

uint32_t listbox_selected_text_color() {
    return listbox_selected_text_color_;
}

uint32_t scrollbox_margin() {
    return scrollbox_margin_;
}

uint32_t scrollbox_scrollbar_size() {
    return scrollbox_scrollbar_size_;
}

uint32_t scrollbox_color() {
    return scrollbox_color_;
}

uint32_t scrollbox_outline_color() {
    return scrollbox_outline_color_;
}

uint32_t editor_line_spacing() {
    return editor_line_spacing_;
}

uint32_t editor_margin() {
    return editor_margin_;
}

uint32_t modal_fill_color() {
    return modal_fill_color_;
}
    
} // namespace theme
} // namespace dfe_ui
