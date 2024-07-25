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

};

} // namespace dfe_ui
