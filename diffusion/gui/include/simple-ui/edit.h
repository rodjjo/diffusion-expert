#pragma once

#include <functional>
#include <string>
#include "simple-ui/component.h"


namespace dfe_ui
{

typedef std::function<void(Component*)> cb_text_changed_t;

class Edit : public Component {
  public:
    Edit(Window * window, int x, int y, int w, int h);
    virtual ~Edit();
    virtual void paint(void *render_window) override;
    bool readonly();
    void readonly(bool value);
    size_t maxlen();
    void maxlen(size_t value);
    void text(const std::wstring& value);
    std::wstring text();
    int character_size();
    void character_size(int value);
    bool password();
    vertical_text_alignment_t text_valign();
    void text_valign(vertical_text_alignment_t value);
    void password(bool value);
    void cursor_color(uint32_t color);
    void text_color(uint32_t color);
    void selection_color(uint32_t color);

    uint32_t cursor_color();
    uint32_t text_color();
    uint32_t selection_color();
    component_cursor_t cursor() override;

  protected:
    bool clickable() override;
    bool editable() override;
    bool focusable() override;
    void handle_textentered(wchar_t unicode) override;
    void handle_keypressed(int key) override;
    void delete_pressed(bool shift_pressed);
    void handle_focus_lost() override;  
    void handle_focus_got() override;

    void handle_mouse_left_pressed(int x, int y) override;
    void handle_mouse_left_released(int x, int y) override;
    void handle_mouse_moved(int x, int y) override;


  private:
    void update_font_min_y_coord();
    void clear_selection();
    void text_changed();
    void select_all();
    void copy_to_clipboard();
    void past_from_clipboard();
    void adjust_selection();
    void backspace_pressed(bool control_pressed);
    void left_pressed(bool shift);
    void right_pressed(bool shift);
    void home_pressed(bool shift);
    void end_pressed(bool shift);
    void get_selection_area(int& x1, int& x2);
    int get_insert_coord();
    int get_character_pos_at_coord(int x);

  private:
    cb_text_changed_t cb_text_changed_;
    size_t  maxlen_ = 0;
    vertical_text_alignment_t    text_valign_ = text_alignment_middle;
    bool    mouse_down_ = false;
    bool    readonly_ = false;
    bool    focused_ = false;
    bool    password_ = false;
    int     text_min_y_ = 0;
    int     character_size_ = 30;
    size_t  sel_start_ = 0;
    size_t  sel_end_ = 0;
    size_t  cursor_pos_ = 0;
    int     mouse_down_x_ = 0;
    uint32_t text_color_ = RGBA_TO_COLOR(255, 255, 255, 255);
    uint32_t selection_color_ = RGBA_TO_COLOR(64, 64, 64, 128);
    uint32_t cursor_color_ = RGBA_TO_COLOR(255, 255, 255, 255);
    std::shared_ptr<void> text_;
};
    
} // namespace dfe_ui
