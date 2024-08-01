#include <stdio.h>
#include "simple-ui/scrollbar.h"
#include "simple-ui/theme.h"
#include "simple-ui/clock.h"
#include "drawings.h"


namespace dfe_ui
{

Scrollbar::Scrollbar(Window * window, int x, int y, int w, int h, bool vertical) : Component(window), m_vertical(vertical) {
    coordinates(x, y, w, h);
    arrow_color(theme::button_text_color());
    color(theme::button_fill_color());
    outline_color(theme::button_outline_color());
    highlighted_color(theme::button_highlighted_color());
    pressed_color(theme::button_pressed_color());
    scroll_color(theme::scroolbar_color());
    scroll_highlighted_color(theme::scroolbar_highlighted_color());
}

Scrollbar::~Scrollbar() {

}

int Scrollbar::min() {
    return m_min;
}

void Scrollbar::adjust_scroll() {
    int temp;
    if (m_min > m_max) {
        temp = m_max;
        m_max = m_min;
        m_min = temp;
    }
    if (m_value < m_min) {
        m_value = m_min;
    }
    if (m_value > m_max) {
        m_value = m_max;
    }
    if (m_onchange) {
        m_onchange(this);
    }
}

void Scrollbar::min(int value) {
    m_min = value;
    adjust_scroll();
}

int Scrollbar::value() {
    return m_value;
}

void Scrollbar::value(int val) {
    m_value = val;
    adjust_scroll();
}

int Scrollbar::max() {
    return m_max;
}

void Scrollbar::max(int value) {
    m_max = value;
    adjust_scroll();
}

bool Scrollbar::vertical() {
    return m_vertical;
}

void Scrollbar::vertical(bool value) {
    m_vertical = value;
}

void Scrollbar::arrow_color(uint32_t value) {
    m_arrow_color = value;
}

uint32_t Scrollbar::arrow_color() {
    return m_arrow_color;
}

void Scrollbar::color(uint32_t value) {
    m_color = value;
}

uint32_t Scrollbar::color() {
    return m_color;
}

void Scrollbar::outline_color(uint32_t value) {
    m_outline_color = value;
}

uint32_t Scrollbar::outline_color() {
    return m_outline_color;
}

void Scrollbar::highlighted_color(uint32_t color) {
    m_highlighted_color = color;
}

uint32_t Scrollbar::highlighted_color() {
    return m_highlighted_color;
}

uint32_t Scrollbar::pressed_color() {
    return m_pressed_color;
}

void Scrollbar::pressed_color(uint32_t value) {
    m_pressed_color = value;
}

void Scrollbar::handle_mouse_left_pressed(int x, int y) {
    m_mouse_pressed = true;

    compute_mouse_region(x, y);
    m_mouse_down_region = m_mouse_in_region;

    if (m_mouse_in_region == 2)  {
        m_mouse_down_value = m_value;
        if (m_vertical) {
            m_mouse_down_coord = y;
        } else {
            m_mouse_down_coord = x;
        }
    } 

    m_mouse_down_time = 0;
    click_scroll();
    m_mouse_down_time = clock::current_microseconds() + 500000;
}

void Scrollbar::handle_mouse_left_released(int x, int y) {
    m_mouse_pressed = false;
    m_mouse_down_coord = 0;
    m_mouse_down_region = -1;
}

void Scrollbar::mouse_enter() {
    m_mouse_inside = true;
}

void Scrollbar::mouse_exit() {
    m_mouse_inside = false;
    m_mouse_in_region = -1;
}

bool Scrollbar::clickable() {
    return true;
}

component_cursor_t Scrollbar::cursor() {
    return m_current_cursor;
}

void Scrollbar::compute_regions(scrool_regions_t &regions) {
    float scale = abs_scale();
    auto x = (m_vertical ? w() : h());
    auto y = (m_vertical ? h() : w());
    
    int button_size;
    int barsize;
    
    button_size = x;
    barsize = (y - button_size * 2);

    if (barsize < 0) {
        barsize = 0;
    }
    if (button_size < 0) {
        button_size = 0;
    }

    int progess_max = (m_max - m_min);
    int slidersize = barsize - button_size * (m_max - m_min);
    if (slidersize < button_size) {
        slidersize = button_size;
    }
    int progress_area = barsize - slidersize;
    if (progess_max < 1) {
        progess_max = 1;
    }
    double progress_factor = (double)progress_area / (double) progess_max;
    int progress = m_value - m_min;
    int scroll_value = progress_factor * progress;
    
    regions[0].x = 0;
    regions[0].y = 0;
    regions[0].w = button_size;
    regions[0].h = button_size;

    regions[4].x = 0;
    regions[4].y = y - button_size;
    regions[4].w = button_size;
    regions[4].h = button_size;
    
    regions[1].x = 0;
    regions[1].y = button_size;
    regions[1].w = button_size;
    regions[1].h = scroll_value;

    regions[2].x = 0;
    regions[2].y = regions[1].h + regions[1].y;
    regions[2].w = button_size;
    regions[2].h = slidersize;

    regions[3].x = 0;
    regions[3].y = regions[2].h + regions[2].y;
    regions[3].w = button_size;
    regions[3].h = regions[4].y - regions[3].y;
    
    if (!m_vertical) {
        int tmp;
        for (int i = 0; i < 5; i++) {
            tmp = regions[i].x;
            regions[i].x = regions[i].y;
            regions[i].y = tmp;
            tmp = regions[i].h;
            regions[i].h = regions[i].w;
            regions[i].w = tmp;
        }
    }
}

void Scrollbar::compute_mouse_region(int x, int y) {
    scrool_regions_t regions;
    compute_regions(regions);
    m_mouse_in_region = -1;
    for (int i = 0; i < 5; i++) {
        if (x >= regions[i].x && y >= regions[i].y &&
            x <= regions[i].x + regions[i].w && y <= regions[i].y + regions[i].h)  {
            m_mouse_in_region = i;
            break;
        }
    }
}

void Scrollbar::handle_mouse_wheel(int8_t direction, int x, int y) {
    if (direction < 0) {
        value(value() + m_page_size);
    } else {
        value(value() - m_page_size);
    }
}

void Scrollbar::handle_mouse_moved(int x, int y) {
    compute_mouse_region(x, y);
    scrool_regions_t regions;
    compute_regions(regions);

    if (m_mouse_in_region == 0 || m_mouse_in_region == 4) {
        m_current_cursor = cursor_hand;
    } else if (m_mouse_in_region == 2) {
        m_current_cursor = cursor_drag;
    } else {
        m_current_cursor = cursor_arrow;
    }

    if (m_mouse_down_region == 2) {
        int max = m_max - m_min;
        int distance = m_vertical ? y - m_mouse_down_coord : x - m_mouse_down_coord;
        int size = 0;
        if (m_vertical) {
            size = (regions[4].y - regions[0].h) - regions[2].h;
        } else {
            size = (regions[4].x - regions[0].w) - regions[2].w;
        }
        if (size != 0) {
            double value_per_pixel = max / (double)size;
            m_value = m_mouse_down_value + (distance * value_per_pixel);
            adjust_scroll();
        }
    } 
}

void Scrollbar::scroll_color(uint32_t value) {
    m_scroll_color = value;
}

uint32_t Scrollbar::scroll_color() {
    return m_scroll_color;
}

void Scrollbar::scroll_highlighted_color(uint32_t value) {
    m_scroll_highlighted_color = value;
}

uint32_t Scrollbar::scroll_highlighted_color() {
    return m_scroll_highlighted_color;
}

int Scrollbar::page_size() {
    return m_page_size;
}

void Scrollbar::page_size(int value) {
    m_page_size = value;
}

void Scrollbar::onchange(component_event_t value) {
    m_onchange = value;
}

component_event_t Scrollbar::onchange() {
    return m_onchange;
}

void Scrollbar::click_scroll() {
    if (!m_mouse_pressed) return;
    if (m_mouse_down_time > clock::current_microseconds()) return;
    m_mouse_down_time = clock::current_microseconds() + 125000; // 4 

    switch (m_mouse_down_region) {
        case 0:
            value(value() - 1);
        break;

        case 1:
            value(value() - m_page_size);
        break;

        case 3:
            value(value() + m_page_size);
        break;

        case 4:
            value(value() + 1);
        break;

        default:
        break;
    }

}

void Scrollbar::paint(sf::RenderTarget *render_target) {
    click_scroll();

    scrool_regions_t regions;
    compute_regions(regions);

    bool pressed = false;
    bool focused = false;
    float scale = abs_scale();
    const int draw_order[] = {1, 3, 2, 0, 4};
    int reg = 0;

    for (int i = 0; i < 5; i++) { 
        reg = draw_order[i];
        regions[reg].h *= abs_scale();
        regions[reg].w *= abs_scale();
        regions[reg].x *= abs_scale();
        regions[reg].y *= abs_scale();
        focused = m_mouse_in_region == reg;
        pressed = m_mouse_pressed && focused;

        if (reg == 2) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(m_outline_color);

            if (pressed) {
                dw.color(m_arrow_color);
            } else if (focused) {
                dw.color(m_highlighted_color);
            } else {
                dw.color(m_color);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_target); 
        }

        if (reg == 1 || reg == 3) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(m_scroll_color);

            if (pressed) {
                dw.color(m_scroll_highlighted_color);
            } else if (focused) {
                dw.color(m_scroll_highlighted_color);
            } else {
                dw.color(m_scroll_color);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_target); 
        }

        // draw button boxes
        if (reg == 0 || reg == 4) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(m_outline_color);

            if (pressed) {
                dw.color(m_arrow_color);
            } else if (focused) {
                dw.color(m_highlighted_color);
            } else {
                dw.color(m_color);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_target); 
            
            Drawing::Direction dir = Drawing::direction_top;
            if (reg == 0) {
                if(m_vertical) {
                    dir = Drawing::direction_top;
                } else {
                    dir = Drawing::direction_left;
                }
            } else {
                if(m_vertical) {
                    dir = Drawing::direction_bottom;
                } else {
                    dir = Drawing::direction_right;
                }
            }
            int margin = (theme::icon_margin() * scale);
            Drawing ar(Drawing::drawing_arrow, dir);
            ar.outline_color(pressed ? m_pressed_color : m_arrow_color );
            ar.color(pressed ? m_pressed_color : m_arrow_color);
            ar.size(regions[reg].w - margin * 2, regions[reg].h - margin * 2);
            ar.position(abs_x() + margin + regions[reg].x, abs_y() + margin + regions[reg].y);
            ar.margin(0);
            ar.draw(render_target);
        }
    }
}



} // namespace dfe_ui
