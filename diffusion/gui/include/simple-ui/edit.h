#pragma once

#include <vector>
#include <functional>
#include <string>

#include "simple-ui/events.h"
#include "simple-ui/component.h"



namespace dfe_ui
{

class Edit : public Component {
  public:
    Edit(Window * window, int x, int y, int w, int h);
    virtual ~Edit();
    virtual void paint(sf::RenderTarget *render_target) override;
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
    bool line_wrap_enabled();
    void line_wrap_enabled(bool value);

  protected:
    bool clickable() override;
    bool editable() override;
    bool focusable() override;
    void handle_text_entered(wchar_t unicode) override;
    void handle_keypressed(int key) override;
    void delete_pressed(bool shift_pressed);
    void handle_focus_lost() override;  
    void handle_focus_got() override;

    void handle_mouse_left_pressed(int x, int y) override;
    void handle_mouse_left_released(int x, int y) override;
    void handle_mouse_moved(int x, int y) override;


  private:
    void compute_wrap();
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
    float                         m_last_scale = 0.0;
    bool                          m_line_wrap_enabled = false;
    component_event_t             m_cb_text_changed;
    size_t                        m_maxlen = 0;
    vertical_text_alignment_t     m_text_valign = text_alignment_middle;
    bool                          m_mouse_down = false;
    bool                          m_readonly = false;
    bool                          m_focused = false;
    bool                          m_password = false;
    int                           m_text_min_y = 0;
    int                           m_character_size = 30;
    size_t                        m_sel_start = 0;
    size_t                        m_sel_end = 0;
    size_t                        m_cursor_pos = 0;
    int                           m_mouse_down_x = 0;
    uint32_t                      m_text_color = RGBA_TO_COLOR(255, 255, 255, 255);
    uint32_t                      m_selection_color = RGBA_TO_COLOR(64, 64, 64, 128);
    uint32_t                      m_cursor_color = RGBA_TO_COLOR(255, 255, 255, 255);
    std::shared_ptr<void>         m_text;
    std::vector<int>              m_lines_offset;
};
    
} // namespace dfe_ui
