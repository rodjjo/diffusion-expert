#pragma once

#include <inttypes.h>

namespace dfe_ui {
namespace theme {

// editor
uint32_t editor_text_color();
uint32_t editor_outline_color();
uint32_t editor_fill_color();
uint32_t editor_cursor_color();
uint32_t editor_selection_color();
void     editor_color(uint32_t text_color, uint32_t fill_color, uint32_t outline_color, 
    uint32_t cursor_color, uint32_t selection_color);
uint32_t editor_line_spacing();
uint32_t editor_margin();

// label
uint32_t label_text_color();
void label_text_color(uint32_t value);

// button
uint32_t button_text_color();
uint32_t button_fill_color();
uint32_t button_outline_color();
uint32_t button_highlighted_color();
uint32_t button_pressed_color();

void button_color(uint32_t text, uint32_t fill, uint32_t outline, uint32_t highlighted, uint32_t pressed);

// icons
uint8_t icon_margin();
void icon_margin(uint8_t value);

// scrollbar
uint32_t scroolbar_color();
uint32_t scroolbar_highlighted_color();

// progressbar
uint32_t progress_bar_color();
uint32_t progress_bar_outline_color();
uint32_t progress_bar_text_color();
uint32_t progress_bar_progress_color();

// listbox
uint8_t listbox_item_margin();
uint32_t listbox_selected_color();
uint32_t listbox_selected_text_color();

// scrollbox
uint32_t scrollbox_margin();    
uint32_t scrollbox_scrollbar_size();
uint32_t scrollbox_color();
uint32_t scrollbox_outline_color();

// modal
uint32_t modal_fill_color();

};

} // namespace dfe_ui
