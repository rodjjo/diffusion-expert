#include <stdio.h>
#include "simple-ui/scrollbar.h"
#include "simple-ui/theme.h"
#include "simple-ui/clock.h"
#include "drawings.h"


namespace dfe_ui
{

Scrollbar::Scrollbar(int x, int y, int w, int h, bool vertical) : vertical_(vertical) {
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
    return min_;
}

void Scrollbar::adjust_scroll() {
    int temp;
    if (min_ > max_) {
        temp = max_;
        max_ = min_;
        min_ = temp;
    }
    if (value_ < min_) {
        value_ = min_;
    }
    if (value_ > max_) {
        value_ = max_;
    }
}

void Scrollbar::min(int value) {
    min_ = value;
    adjust_scroll();
}

int Scrollbar::value() {
    return value_;
}

void Scrollbar::value(int val) {
    value_ = val;
    adjust_scroll();
}

int Scrollbar::max() {
    return max_;
}

void Scrollbar::max(int value) {
    max_ = value;
    adjust_scroll();
}

bool Scrollbar::vertical() {
    return vertical_;
}

void Scrollbar::vertical(bool value) {
    vertical_ = value;
}

void Scrollbar::arrow_color(uint32_t value) {
    arrow_color_ = value;
}

uint32_t Scrollbar::arrow_color() {
    return arrow_color_;
}

void Scrollbar::color(uint32_t value) {
    color_ = value;
}

uint32_t Scrollbar::color() {
    return color_;
}

void Scrollbar::outline_color(uint32_t value) {
    outline_color_ = value;
}

uint32_t Scrollbar::outline_color() {
    return outline_color_;
}

void Scrollbar::highlighted_color(uint32_t color) {
    highlighted_color_ = color;
}

uint32_t Scrollbar::highlighted_color() {
    return highlighted_color_;
}

uint32_t Scrollbar::pressed_color() {
    return pressed_color_;
}

void Scrollbar::pressed_color(uint32_t value) {
    pressed_color_ = value;
}

void Scrollbar::handle_mouse_left_pressed(int x, int y) {
    mouse_pressed_ = true;

    compute_mouse_region(x, y);
    mouse_down_region_ = mouse_in_region_;

    if (mouse_in_region_ == 2)  {
        mouse_down_value_ = value_;
        if (vertical_) {
            mouse_down_coord_ = y;
        } else {
            mouse_down_coord_ = x;
        }
    } 

    mouse_down_time_ = 0;
    click_scroll();
    mouse_down_time_ = clock::current_microseconds() + 500000;
}

void Scrollbar::handle_mouse_left_released(int x, int y) {
    mouse_pressed_ = false;
    mouse_down_coord_ = 0;
    mouse_down_region_ = -1;
}

void Scrollbar::mouse_enter() {
    mouse_inside_ = true;
}

void Scrollbar::mouse_exit() {
    mouse_inside_ = false;
    mouse_in_region_ = -1;
}

bool Scrollbar::clickable() {
    return true;
}

component_cursor_t Scrollbar::cursor() {
    return current_cursor_;
}

void Scrollbar::compute_regions(scrool_regions_t &regions) {
    float scale = abs_scale();
    auto x = (vertical_ ? w() : h()) * scale;
    auto y = (vertical_ ? h() : w()) * scale;
    
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

    int progess_max = (max_ - min_);
    int slidersize = barsize - button_size * (max_ - min_);
    if (slidersize < button_size) {
        slidersize = button_size;
    }
    int progress_area = barsize - slidersize;
    if (progess_max < 1) {
        progess_max = 1;
    }
    double progress_factor = (double)progress_area / (double) progess_max;
    int progress = value_ - min_;
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
    
    if (!vertical_) {
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
    mouse_in_region_ = -1;
    for (int i = 0; i < 5; i++) {
        if (x >= regions[i].x && y >= regions[i].y &&
            x <= regions[i].x + regions[i].w && y <= regions[i].y + regions[i].h)  {
            mouse_in_region_ = i;
            break;
        }
    }
}

void Scrollbar::handle_mouse_wheel(int8_t direction, int x, int y) {
    if (direction < 0) {
        value(value() + page_size_);
    } else {
        value(value() - page_size_);
    }
}

void Scrollbar::handle_mouse_moved(int x, int y) {
    compute_mouse_region(x, y);
    scrool_regions_t regions;
    compute_regions(regions);
    
    if (mouse_in_region_ == 0 || mouse_in_region_ == 4) {
        current_cursor_ = cursor_hand;
    } else if (mouse_in_region_ == 2) {
        current_cursor_ = cursor_drag;
    } else {
        current_cursor_ = cursor_arrow;
    }

    if (mouse_down_region_ == 2) {
        int max = max_ - min_;
        int distance = vertical_ ? y - mouse_down_coord_ : x - mouse_down_coord_;
        int size = 0;
        if (vertical_) {
            size = (regions[4].y - regions[0].h) - regions[2].h;
        } else {
            size = (regions[4].x - regions[0].w) - regions[2].w;
        }
        if (size != 0) {
            double value_per_pixel = max / (double)size;
            value_ = mouse_down_value_ + (distance * value_per_pixel);
            adjust_scroll();
        }
    } 
}

void Scrollbar::scroll_color(uint32_t value) {
    scroll_color_ = value;
}

uint32_t Scrollbar::scroll_color() {
    return scroll_color_;
}

void Scrollbar::scroll_highlighted_color(uint32_t value) {
    scroll_highlighted_color_ = value;
}

uint32_t Scrollbar::scroll_highlighted_color() {
    return scroll_highlighted_color_;
}

int Scrollbar::page_size() {
    return page_size_;
}

void Scrollbar::page_size(int value) {
    page_size_ = value;
}

void Scrollbar::click_scroll() {
    if (!mouse_pressed_) return;
    if (mouse_down_time_ > clock::current_microseconds()) return;
    mouse_down_time_ = clock::current_microseconds() + 125000; // 4 

    switch (mouse_down_region_) {
        case 0:
            value(value() - 1);
        break;

        case 1:
            value(value() - page_size_);
        break;

        case 3:
            value(value() + page_size_);
        break;

        case 4:
            value(value() + 1);
        break;

        default:
        break;
    }

}

void Scrollbar::paint(void *render_window) {
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
        focused = mouse_in_region_ == reg;
        pressed = mouse_pressed_ && focused;

        if (reg == 2) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(outline_color_);

            if (pressed) {
                dw.color(arrow_color_);
            } else if (focused) {
                dw.color(highlighted_color_);
            } else {
                dw.color(color_);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_window); 
        }

        if (reg == 1 || reg == 3) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(scroll_color_);

            if (pressed) {
                dw.color(scroll_highlighted_color_);
            } else if (focused) {
                dw.color(scroll_highlighted_color_);
            } else {
                dw.color(scroll_color_);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_window); 
        }

        // draw button boxes
        if (reg == 0 || reg == 4) {
            Drawing dw(Drawing::drawing_flat_box);
            dw.outline_color(outline_color_);

            if (pressed) {
                dw.color(arrow_color_);
            } else if (focused) {
                dw.color(highlighted_color_);
            } else {
                dw.color(color_);  
            }
            
            dw.size(regions[reg].w, regions[reg].h);
            dw.position(abs_x() + regions[reg].x, abs_y() + regions[reg].y);
            dw.margin(0);
            dw.draw(render_window); 
            
            Drawing::Direction dir = Drawing::direction_top;
            if (reg == 0) {
                if(vertical_) {
                    dir = Drawing::direction_top;
                } else {
                    dir = Drawing::direction_left;
                }
            } else {
                if(vertical_) {
                    dir = Drawing::direction_bottom;
                } else {
                    dir = Drawing::direction_right;
                }
            }
            int margin = (theme::icon_margin() * scale);
            Drawing ar(Drawing::drawing_arrow, dir);
            ar.outline_color(pressed ? pressed_color_ : arrow_color_ );
            ar.color(pressed ? pressed_color_ : arrow_color_);
            ar.size(regions[reg].w - margin * 2, regions[reg].h - margin * 2);
            ar.position(abs_x() + margin + regions[reg].x, abs_y() + margin + regions[reg].y);
            ar.margin(0);
            ar.draw(render_window);
        }
    }
}



} // namespace dfe_ui
