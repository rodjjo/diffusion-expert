#include "simple-ui/theme.h"

namespace dfe_ui {
namespace theme {
namespace {
    // editor
    uint32_t m_editor_text_color             = 0x000000FF;
    uint32_t m_editor_out_color              = 0xFF0000FF;
    uint32_t m_editor_fill_color             = 0xFFFFFFFF;
    uint32_t m_editor_cursor_color           = 0x000000FF;
    uint32_t m_editor_selection_color        = 0x11111133;

    // button
    uint32_t m_button_text_color             = 0x111111FF;
    uint32_t m_button_fill_color             = 0x444444FF;
    uint32_t m_button_pressed_color          = 0x555555FF;
    uint32_t m_button_out_color              = 0xFF0000FF;
    uint32_t m_button_highlighted_color      = 0x222222FF;

    // label
    uint32_t m_label_text_color              = 0x000000FF;

    // icons
    uint8_t m_icon_margin                    = 5;

    // scrollbar
    uint32_t m_scrollbar_color               = 0xAAAAAAFF;
    uint32_t m_scrollbar_highlighted_color   = 0xBBBBBBFF;

    // progressbar
    uint32_t m_progress_bar_color            = 0xFFFFFFFF;
    uint32_t m_progress_bar_outline_color    = 0x000000FF;
    uint32_t m_progress_bar_text_color       = 0x888888FF;
    uint32_t m_progress_bar_progress_color   = 0x1100DDFF;

    // listbox
    uint8_t m_listbox_item_margin             = 3;
    uint32_t m_listbox_selected_color         = 0x000000FF;
    uint32_t m_listbox_selected_text_color    = 0xFFFFFFFF;

    // scrollbox
    uint32_t m_scrollbox_margin               = 10;    
    uint32_t m_scrollbox_scrollbar_size       = 20;
    uint32_t m_scrollbox_color                = 0xBCBCBCFF;
    uint32_t m_scrollbox_outline_color        = 0xCCCCCCFF;
}

uint32_t editor_text_color() {
    return m_editor_text_color;
}

uint32_t editor_outline_color() {
    return m_editor_out_color;
}

uint32_t editor_fill_color() {
    return m_editor_fill_color;
}

uint32_t editor_cursor_color() {
    return m_editor_cursor_color;
}

uint32_t editor_selection_color() {
    return m_editor_selection_color;
}

uint32_t button_text_color() {
    return m_button_text_color;
}

uint32_t label_text_color() {
    return m_label_text_color;
}

uint32_t button_fill_color() {
    return m_button_fill_color;
}

uint32_t button_outline_color() {
    return m_button_out_color;
}

uint32_t button_pressed_color() {
    return m_button_pressed_color;
}

void editor_color(uint32_t text_color, uint32_t fill_color, uint32_t outline_color, 
    uint32_t cursor_color, uint32_t selection_color) {
    m_editor_text_color = text_color;
    m_editor_cursor_color = cursor_color;
    m_editor_fill_color = fill_color;
    m_editor_out_color = outline_color;
    m_editor_selection_color = selection_color;
}

void button_color(uint32_t text, uint32_t fill, uint32_t outline, uint32_t highlighted, uint32_t pressed){
    m_button_text_color = text;
    m_button_fill_color = fill;
    m_button_out_color = outline;
    m_button_highlighted_color = highlighted;
    m_button_pressed_color = pressed;
}

uint32_t button_highlighted_color() {
    return m_button_highlighted_color;
}

void label_text_color(uint32_t value) {
    m_label_text_color = value;
}


uint8_t icon_margin() {
    return m_icon_margin;
}

void icon_margin(uint8_t value) {
    m_icon_margin = value;
}

uint32_t scroolbar_color() {
    return m_scrollbar_color;
}

uint32_t scroolbar_highlighted_color() {
    return m_scrollbar_highlighted_color;
}

uint32_t progress_bar_color() {
    return m_progress_bar_color;
}

uint32_t progress_bar_outline_color() {
    return m_progress_bar_outline_color;
}

uint32_t progress_bar_text_color() {
    return m_progress_bar_text_color;
}

uint32_t progress_bar_progress_color() {
    return m_progress_bar_progress_color;
}

uint8_t listbox_item_margin() {
    return m_listbox_item_margin;
}

uint32_t listbox_selected_color() {
    return m_listbox_selected_color;
}

uint32_t listbox_selected_text_color() {
    return m_listbox_selected_text_color;
}

uint32_t scrollbox_margin() {
    return m_scrollbox_margin;
}

uint32_t scrollbox_scrollbar_size() {
    return m_scrollbox_scrollbar_size;
}

uint32_t scrollbox_color() {
    return m_scrollbox_color;
}

uint32_t scrollbox_outline_color() {
    return m_scrollbox_outline_color;
}
    
} // namespace theme
} // namespace dfe_ui
