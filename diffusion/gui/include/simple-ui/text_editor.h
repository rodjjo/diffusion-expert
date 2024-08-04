#pragma once

#include <vector>
#include <string>
#include "simple-ui/component.h"


namespace dfe_ui
{

typedef enum {
    editor_text,
    editor_text_wrap,
    editor_multiline,
    editor_multiline_wrap,
    editor_integer,
    editor_float,
    editor_password    
} editor_type_t; 

typedef enum {
    text_top,
    text_center,
    text_bottom
} editor_vertical_aligment_t;

typedef enum {
    text_left,
    text_right
} editor_horizontal_aligment_t;

typedef enum {
    label_top,
    label_left,
    label_place_holder,
    label_inside
} editor_label_position_t;


class TextEditor : public Component {
    public:
        TextEditor(Window * window, int x, int y, int w, int h, editor_type_t type);
        virtual ~TextEditor(){};
        virtual void paint(sf::RenderTarget *render_target) override;

        // colors
        void fill_color(uint32_t value);
        uint32_t fill_color();
        void outline_color(uint32_t value);
        uint32_t outline_color();
        void text_color(uint32_t value);
        uint32_t text_color();
        void cursor_color(uint32_t value);
        uint32_t cursor_color();
        void selected_color(uint32_t value);
        uint32_t selected_color();

        // aligment
        editor_vertical_aligment_t valign();
        void halign(editor_vertical_aligment_t value);
        editor_horizontal_aligment_t halign();
        void halign(editor_horizontal_aligment_t value);

        // content
        std::wstring get_selected_text();
        std::wstring content();
        void content(const std::wstring &value);
        double float_content();
        int64_t integer_content();

    private:
        bool clickable() override;
        bool editable() override;
        bool focusable() override;
        void handle_text_entered(wchar_t unicode) override;
        void handle_keypressed(int key) override;
        void handle_focus_lost() override;  
        void handle_focus_got() override;

        void handle_mouse_left_pressed(int x, int y) override;
        void handle_mouse_left_released(int x, int y) override;
        void handle_mouse_moved(int x, int y) override;

    private:
        sf::Text *text_display(size_t index);
        size_t cursor_x();
        size_t cursor_y();
        void select_all();
        void select_nothing();
        void remove_selected_text();
        void copy_to_clipboard();
        void paste_from_clipboard();
        void delete_pressed();
        void backspace_pressed();
        void left_pressed();
        void right_pressed();
        void up_pressed();
        void down_pressed();
        void home_pressed();
        void end_pressed();
        void enter_pressed();
        bool is_control_pressed();
        bool is_shift_pressed();
        void fix_selection();
        bool has_selection();
        void begin_selection();
        void end_selection();
        void wrap_text();
        void apply_format(sf::Text *txt);
        size_t unwrap_cursor(size_t x, size_t y);
        size_t unwrap_cursor();
        void wrap_cursor(size_t value, size_t *x, size_t *y);
        void wrap_cursor(size_t value);
        wchar_t latest_character(size_t line_number);
        void insert_character(wchar_t unicode);
        bool update_measurement_item();
        std::pair<size_t, size_t> find_cursor_from_mouse_coords(int x, int y);

    private:
        size_t m_scroll_top = 0;
        size_t m_cursor_x = 0;
        size_t m_cursor_y = 0;
        size_t m_selection_x1 = 0;
        size_t m_selection_y1 = 0;
        size_t m_selection_x2 = 0;
        size_t m_selection_y2 = 0;
        size_t m_sel_started_at = 0;
    private:
        bool m_readonly = false;
        bool m_need_update = true;
        bool m_focused = false;
        bool m_mouse_left_pressed = false;
        std::vector<std::wstring> m_lines;
        std::shared_ptr<sf::Text>  m_text_measure;
        std::vector<std::shared_ptr<sf::Text> > m_texts;
        editor_vertical_aligment_t m_valign = text_center;
        editor_horizontal_aligment_t m_halign = text_left;
        editor_type_t m_type = editor_type_t::editor_text;
        int m_character_size = 30;
        uint32_t m_fill_color = 0;
        uint32_t m_outline_color = 0;
        uint32_t m_text_color = 0;
        uint32_t m_cursor_color = 0;
        uint32_t m_selected_color = 0;
};

} // namespace dfe_ui
