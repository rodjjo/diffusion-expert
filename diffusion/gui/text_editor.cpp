#include <sstream>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Graphics/Text.hpp>

#include "drawings.h"
#include "simple-ui/clock.h"
#include "simple-ui/theme.h"
#include "simple-ui/default_font.h"

#include "simple-ui/text_editor.h"


namespace dfe_ui
{

TextEditor::TextEditor(Window * window, int x, int y, int w, int h, editor_type_t type) : Component(window), m_type(type) {
    text_color(dfe_ui::theme::editor_text_color());
    cursor_color(dfe_ui::theme::editor_cursor_color());
    selected_color(dfe_ui::theme::editor_selection_color());
    outline_color(dfe_ui::theme::editor_outline_color());
    fill_color(dfe_ui::theme::editor_fill_color());
    m_vscrollbar.reset(new Scrollbar(window, 0, 0, theme::scrollbox_scrollbar_size(), h, true));
    m_hscrollbar.reset(new Scrollbar(window, 0, 0, w, theme::scrollbox_scrollbar_size(), false));
    m_vscrollbar->visible(this->m_vertical_scrollbar == scrollbar_allways);
    m_hscrollbar->visible(this->m_horizontal_scrollbar == scrollbar_allways);
    add(m_vscrollbar);
    add(m_hscrollbar);
    this->coordinates(x, y, w, h);
    m_vscrollbar->onchange([this](Component *self) {
        vertical_scrollbar_changed();
    });
}


void TextEditor::fill_color(uint32_t value) {
    m_fill_color = value;
}

uint32_t TextEditor::fill_color(){
    return m_fill_color;
}

void TextEditor::outline_color(uint32_t value){
    m_outline_color = value;
}

uint32_t TextEditor::outline_color(){
    return m_outline_color;
}

void TextEditor::text_color(uint32_t value){
    m_text_color = value;
}

uint32_t TextEditor::text_color() {
    return m_text_color;
}

void TextEditor::cursor_color(uint32_t value) {
    m_cursor_color = value;
}

uint32_t TextEditor::cursor_color() {
    return m_cursor_color;
}

void TextEditor::selected_color(uint32_t value) {
    m_selected_color = value;
}

uint32_t TextEditor::selected_color() {
    return m_selected_color;
}

editor_horizontal_aligment_t TextEditor::halign() {
    return m_halign;
}

void TextEditor::halign(editor_horizontal_aligment_t value) {
    m_halign = value;
}

size_t TextEditor::cursor_x() {
     return m_cursor_x;   
}

size_t TextEditor::cursor_y() {
    return m_cursor_y;
}

bool TextEditor::clickable() {
    return true;
}

bool TextEditor::editable() {
    return true;
}

bool TextEditor::focusable() {
    return true;
}

void TextEditor::remove_selected_text() {
    if (!has_selection()) return;
    fix_selection();
    auto sel_start = unwrap_cursor(m_selection_x1, m_selection_y1);
    auto sel_end = unwrap_cursor(m_selection_x2, m_selection_y2);
    auto content = this->content();
    if (sel_start < content.size()) {
        content.erase(sel_start, sel_end - sel_start);
    }
    this->content(content);
    wrap_cursor(sel_start);
    select_nothing();
    wrap_text();
}

void TextEditor::handle_text_entered(wchar_t unicode) {
    if (m_readonly) return;

    char c[MB_CUR_MAX];
    int len = wctomb(c, unicode);
    if (len > 0) {
        unsigned char u = static_cast<unsigned char>(c[0]);
        if (u < 32 || u > 126) {
            return;
        }
    }

    insert_character(unicode);
    update_scroll();
}

void TextEditor::insert_character(wchar_t unicode) {
    if (m_type == editor_integer || m_type == editor_float) {
        if ((unicode < u'0' || unicode > u'9') && unicode != u'-' && unicode != u'.') {
            return;
        }
    }
    if (m_type == editor_integer && unicode == u'.') {
        return;
    }
    if (m_type == editor_float && unicode == u'.') {
        if (content().find_first_of(u'.') != std::wstring::npos) {
            return;
        }
        if (content().empty() || content() == L"-") {
            insert_character(u'0');
        }
    }
    if ((m_type == editor_integer || m_type == editor_float) && unicode == u'-') {
        if (m_cursor_x != 0 || content().find_first_of(u'-') != std::wstring::npos) {
            return;
        }
    }
    
    remove_selected_text();

    if (m_lines.size() < 1) {
        m_lines.push_back(L"");
    }
    if (m_cursor_y > m_lines.size()) {
        m_cursor_y = m_lines.size() - 1;
    }
    if (m_cursor_x > m_lines[m_cursor_y].size()) {
        m_cursor_x = m_lines[m_cursor_y].size();
    }
    m_lines[m_cursor_y].insert(m_lines[m_cursor_y].begin() + m_cursor_x, unicode);
    m_cursor_x++;
    
    m_need_update = true;
    wrap_text();
}

bool TextEditor::is_control_pressed() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RControl);
}

bool TextEditor::is_shift_pressed() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RShift);
}

void TextEditor::handle_keypressed(int key) {
    switch ((sf::Keyboard::Key)key) {
        case sf::Keyboard::Key::A:
            if (is_control_pressed())
                select_all();
            break;
            case sf::Keyboard::Key::C:
            if (is_control_pressed())
                copy_to_clipboard();
            break;
            case sf::Keyboard::Key::V:
            if (is_control_pressed())
                paste_from_clipboard();
            break;
            case sf::Keyboard::Key::X:
            if (is_control_pressed()) {
                copy_to_clipboard();
                remove_selected_text();
            }
            break;

            case sf::Keyboard::Key::Up:
                up_pressed();
            break;
            case sf::Keyboard::Key::Down:
                down_pressed();
            break;
            case sf::Keyboard::Key::Enter:
                enter_pressed();
            break;
            case sf::Keyboard::Key::Tab:
                //if (m_listener)
                // m_listener->fireEditSpecKeyPressed(ev);
            //extended key
            break;

            case sf::Keyboard::Key::Backspace:
                backspace_pressed();
            break;

            case sf::Keyboard::Key::Delete: delete_pressed(); break;
            case sf::Keyboard::Key::Left: left_pressed(); break;
            case sf::Keyboard::Key::Right: right_pressed(); break;
            case sf::Keyboard::Key::Home: home_pressed(); break;
            case sf::Keyboard::Key::End: end_pressed(); break;

            default:
            break;
    }
}

void TextEditor::backspace_pressed() {
    if (m_readonly || m_lines.empty()) {
        return;
    }

    if (has_selection()) {
        remove_selected_text();
        return;
    }

    auto cx = m_cursor_x;
    auto cy = m_cursor_y;
    left_pressed();
    if (cx == m_cursor_x && cy == m_cursor_y) {
        return;
    }

    if (m_cursor_y < m_lines.size() && m_cursor_x < m_lines[m_cursor_y].size()) {
        m_lines[m_cursor_y].erase(m_cursor_x, 1);
    }
    
    m_need_update = true;
    wrap_text();
}

void TextEditor::enter_pressed() {
    if (m_readonly || (m_type != editor_multiline_wrap && m_type != editor_multiline)) {
        return;
    }

    remove_selected_text();

    if (m_lines.empty()) {
        m_lines.push_back(std::wstring(L"\n"));
        m_cursor_x = 0;
        m_cursor_y = 0;
        update_scroll();
    } else {
        insert_character(U'\n');
        m_cursor_x = 0;
        m_cursor_y++;
        if (m_cursor_y >= m_lines.size()) {
            m_cursor_y = m_lines.size() - 1;
        }
        update_scroll();
    }
}

void TextEditor::left_pressed() {

    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }

    if (m_cursor_x > 0) {
        m_cursor_x--;
    } else if (m_cursor_y > 0) {
        m_cursor_y--;
        if (m_cursor_y < m_lines.size()) {
            m_cursor_x = m_lines[m_cursor_y].size();
            if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
                m_cursor_x--;
            }
        } else {
            m_cursor_x = 0;
        }
    }


    if (is_shift_pressed()) {
        end_selection();
    }
}

void TextEditor::right_pressed() {
    if (m_cursor_y >= m_lines.size() || m_lines.empty()) {
        return;
    }

    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }

    if (m_cursor_x < m_lines[m_cursor_y].size()) {
        if (m_lines[m_cursor_y][m_cursor_x] == U'\n') {
            m_cursor_x = 0;
            m_cursor_y++;
        } else {
            m_cursor_x += 1;
        }
    } else if (m_cursor_y + 1 < m_lines.size()) {
        m_cursor_x = 0;
        m_cursor_y++;
    }

    if (m_cursor_y >= m_lines.size()) {
        m_cursor_y--;
        m_cursor_x = m_lines[m_cursor_y].size();
        if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
            m_cursor_x--;
        }
    }

    if (is_shift_pressed()) {
        end_selection();
    }
}

void TextEditor::up_pressed() {
    if (m_type != editor_multiline && m_type != editor_multiline_wrap && m_type != editor_text_wrap) {
        return;
    }

    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }

    if (m_lines.empty()) {
        m_cursor_y = 0;
        m_cursor_x = 0;
        if (is_shift_pressed()) {
            end_selection();
        }
        update_scroll();
        return;
    }

    if (m_cursor_y > 0) {
        if (m_cursor_y >= m_lines.size()) {
            m_cursor_y = m_lines.size() - 1;
        } else {
            m_cursor_y--;
        }
    }
    if (m_cursor_x > m_lines[m_cursor_y].size()) {
        m_cursor_x = m_lines[m_cursor_y].size();
        if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
            m_cursor_x--;
        }
    }

    if (is_shift_pressed()) {
        end_selection();
    }

    update_scroll();
}

void TextEditor::down_pressed() {
    if (m_type != editor_multiline && m_type != editor_multiline_wrap && m_type != editor_text_wrap) {
        return;
    }

    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }

    if (m_lines.empty()) {
        m_cursor_y = 0;
        m_cursor_x = 0;
        if (is_shift_pressed()) {
            end_selection();
        }
        update_scroll();
        return;
    }

    if (m_cursor_y >= m_lines.size()) {
        m_cursor_y = m_lines.size() - 1;
    } else if (m_cursor_y < m_lines.size() - 1) {
        m_cursor_y++;
    }
    
    if (m_cursor_x > m_lines[m_cursor_y].size()) {
        m_cursor_x = m_lines[m_cursor_y].size();
        if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
            m_cursor_x--;
        }
    }

    if (is_shift_pressed()) {
        end_selection();
    }

    update_scroll();
}

void TextEditor::home_pressed() {
    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }


    m_cursor_x = 0;
    if (is_control_pressed()) {
        m_scroll_top = 0;
        m_cursor_y = 0;
    }

    if (is_shift_pressed()) {
        end_selection();
    }
    
    update_scroll();
}

void TextEditor::end_pressed() {
    if (is_shift_pressed()) {
        begin_selection();
    } else {
        select_nothing();
    }

    if (is_control_pressed()) {
        m_cursor_y = m_lines.size() - 1;
        m_scroll_top = m_lines.size();
    }

    if (m_lines.empty()) {
        if (is_shift_pressed()) {
            end_selection();
        }
        update_scroll();
        return;
    }

    if (m_cursor_y >= m_lines.size()) {
        m_cursor_y--;
        m_cursor_x = m_lines[m_cursor_x].size();
        if (m_cursor_x > 0) {
            m_cursor_x--;
        }
    }

    if (m_cursor_y < m_lines.size()) {
        m_cursor_x = m_lines[m_cursor_y].size();
    } else {
        m_cursor_x = 0;
    }

    if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
        m_cursor_x--;
    }

    if (is_shift_pressed()) {
        end_selection();
    }

    update_scroll();
}

void TextEditor::select_nothing() {
    m_selection_x1 = 0;
    m_selection_y1 = 0;
    m_selection_x2 = 0;
    m_selection_y2 = 0;
    m_sel_started_at = 0;
}

void TextEditor::select_all() {
    select_nothing();
    if (m_lines.size()) {
        m_selection_y2 = m_lines.size() - 1;
        m_selection_x2 = m_lines[m_selection_y2].size() - 1;
        if (m_type != editor_multiline && m_type != editor_multiline_wrap) {
            m_selection_x2++;
        }
        m_cursor_x = m_selection_x2;
        m_cursor_y = m_selection_y2;
    }
}

bool TextEditor::has_selection() {
    return (m_selection_x1 != m_selection_x2 || m_selection_y1 != m_selection_y2);
}

void TextEditor::fix_selection() {
    if (m_lines.empty()) {
        select_nothing();
        return;
    }
    bool multiline = m_type == (editor_multiline || editor_multiline_wrap);

    if (m_selection_y1 >= m_lines.size()) {
        m_selection_y1 = m_lines.size() - 1;
        m_selection_x1  = m_lines[m_selection_y1].size();
        if (m_selection_x1 && !multiline) {
            m_selection_x1--;
        }
    }

    if (m_selection_y2 >= m_lines.size()) {
        m_selection_y2 = m_lines.size() - 1;
        m_selection_x2  = m_lines[m_selection_y2].size();
        if (m_selection_x2 && !multiline) m_selection_x2--;
    }

    if (m_selection_y1 > m_selection_y2) {
        auto tmp = m_selection_y1;
        m_selection_y1 = m_selection_y2;
        m_selection_y2 = tmp;
        tmp = m_selection_x1;
        m_selection_x1 = m_selection_x2;
        m_selection_x2 = tmp;
    } 
   
    if (m_selection_x2 > 0 && m_selection_x2 >= m_lines[m_selection_y2].size() + (multiline ? 0 : 1)) {
        m_selection_x2 = m_lines[m_selection_y2].size() - (multiline ? 1 : 0);
        if (!multiline) m_selection_x2++;
    }
    if (m_selection_x1 > 0 && m_selection_x1 >= m_lines[m_selection_y1].size() + (multiline ? 0 : 1)) {
        m_selection_x1 = m_lines[m_selection_y1].size() - (multiline ? 1 : 0);
    }

    if (m_selection_y1 == m_selection_y2) {
        if (m_selection_x2 < m_selection_x1) {
            auto tmp = m_selection_x2;
            m_selection_x2 = m_selection_x1;
            m_selection_x1 = tmp;
        }
    }

    // printf("Selection x1 %d  x2 %d  Cursor %d\n", (int)m_selection_x1, (int)m_selection_x2, (int)m_cursor_x);
 }

void TextEditor::begin_selection() {
    if (has_selection()) {
        return;
    }
    m_sel_started_at = unwrap_cursor(m_cursor_x, m_cursor_y);
    m_selection_x1 = m_cursor_x;
    m_selection_y1 = m_cursor_y;
    m_selection_x2 = m_cursor_x;
    m_selection_y2 = m_cursor_y;

}

void TextEditor::end_selection() {
    size_t cx = 0, cy = 0;
    wrap_cursor(m_sel_started_at, &cx, &cy);

    m_selection_x1 = cx;
    m_selection_y1 = cy;
    m_selection_x2 = m_cursor_x;
    m_selection_y2 = m_cursor_y;

    fix_selection();
}

std::wstring TextEditor::get_selected_text() {
    fix_selection();
    std::wstring result;
    for (auto y = m_selection_y1; y <= m_selection_y2 && y < m_lines.size(); y++) {
        if (y == m_selection_y1) {
            result += m_lines[y].substr(m_selection_x1);
        } else if (y == m_selection_y2) {
            result += m_lines[y].substr(0, m_selection_x2);
        } else {
            result += m_lines[y];
        }
    }
    return result;
}

void TextEditor::copy_to_clipboard() {
    if (m_type != editor_password) {
        sf::Clipboard::setString(get_selected_text());
    }
}

void TextEditor::paste_from_clipboard() {
    auto value = sf::Clipboard::getString();
    if (!value.isEmpty()) {
        remove_selected_text();
        auto cursor = unwrap_cursor();
        auto content = this->content();
        if (cursor < content.size()) {
            content.insert(cursor, value);
        } else {
            content += value;
        }
        cursor += value.getSize();
        this->content(content);
        wrap_cursor(cursor);
    }
}

void TextEditor::delete_pressed() {
    if (m_lines.empty()) 
        return;

    if (has_selection()) {
        remove_selected_text();
        return;
    }

    if (is_shift_pressed()) {
        backspace_pressed();
        return;
    } else {
        auto cx = m_cursor_x;
        auto cy = m_cursor_y;
        right_pressed();
        if (cx != m_cursor_x || cy != m_cursor_y) {
            backspace_pressed();
        }
    }
}

void TextEditor::handle_focus_lost() {
    m_focused = false;
}

void TextEditor::handle_focus_got() {
    m_focused = true;
}

component_cursor_t TextEditor::cursor() {
    return cursor_edit;
}


void TextEditor::handle_mouse_left_pressed(int x, int y) {
    m_mouse_left_pressed = true;
    begin_selection();
    
    auto cursor = find_cursor_from_mouse_coords(x, y);
    m_cursor_x = cursor.first;
    m_cursor_y = cursor.second;

    if (is_shift_pressed()) {
        end_selection();
    } else {
        select_nothing();
        begin_selection();
    }
}

void TextEditor::handle_mouse_left_released(int x, int y) {
    auto cursor = find_cursor_from_mouse_coords(x, y);
    m_cursor_x = cursor.first;
    m_cursor_y = cursor.second;
    end_selection();
    m_mouse_left_pressed = false;
}

void TextEditor::handle_mouse_moved(int x, int y) {
    if (m_mouse_left_pressed) {
        auto cursor = find_cursor_from_mouse_coords(x, y);
        m_cursor_x = cursor.first;
        m_cursor_y = cursor.second;
        end_selection();
    }
}

void TextEditor::handle_parent_resized() {
    m_vscrollbar->coordinates(
        w() - theme::scrollbox_scrollbar_size(),
        0, theme::scrollbox_scrollbar_size(),
        h() - (m_hscrollbar->visible() ? theme::scrollbox_scrollbar_size() : 0)
    );

    m_hscrollbar->coordinates(
        0, h() - theme::scrollbox_scrollbar_size(),
        w() - (m_vscrollbar->visible() ? theme::scrollbox_scrollbar_size() : 0),
        theme::scrollbox_scrollbar_size()
    );

}

std::pair<size_t, size_t> TextEditor::find_cursor_from_mouse_coords(int x, int y) {
    std::pair<size_t, size_t> result(0, 0);
    if (!update_measurement_item() || m_lines.empty()) {
        return result;
    }

    float scale = abs_scale();
    x *= scale;
    y *= scale;
    int line_height = (m_character_size + theme::editor_line_spacing()) * scale;
    result.second = m_scroll_top + y / line_height;

    if (result.second >= m_lines.size()) {
        result.second = m_lines.size() - 1;
    }

    auto text = m_lines[result.second];
    m_text_measure->setString(m_type == editor_password ? std::wstring(text.size(), U'*') : text);
    
    int left_coord = 0;
    int line_width = (abs_w() - theme::editor_margin() * 2) * scale;
    line_width -= (m_vscrollbar && m_vscrollbar->visible()) ? m_vscrollbar->abs_w() : 0;
    if (m_halign == text_right) {
        auto p = m_text_measure->getLocalBounds();
        if (p.size.x < line_width) {
            left_coord += line_width - p.size.x;
        }
    }
    m_text_measure->setPosition({left_coord, 0});

    size_t distance = (size_t)-1;
    for (size_t i = 0; i < text.size(); i++) {
        auto p = m_text_measure->findCharacterPos(i);
        if (abs((double)p.x - (double)x) < distance) {
            result.first = i;
            distance = abs((double)p.x - (double)x);
        }
    }

    auto p = m_text_measure->getGlobalBounds();
    auto x2 = p.position.x + p.size.x;
    if (abs((double)x2 - (double)x) < distance) {
        result.first = text.size();
    }
    
    if (text.size() > 0 && result.first > 0) {
        if (result.first >= text.size() && (m_type == editor_multiline || m_type == editor_multiline_wrap))  {
            result.first--;
        }
        if (result.first > 0 && text[result.first - 1] == U'\n') {
            result.first--;
        }
    } else {
        result.first = 0;
    }

    m_text_measure->setPosition({0, 0});

    return result;
}


std::wstring TextEditor::content() {
    std::wstring result;
    for (auto & l : m_lines) {
        result += l;
    }
    return result;
}

double TextEditor::float_content() {
    double vdouble = 0;
    if (swscanf(content().c_str(), L"%lf", &vdouble) == 1) {
        return vdouble;
    }
    return 0;
}

int64_t TextEditor::integer_content() {
    uint32_t v64 = 0;
    if (swscanf(content().c_str(), L"%lld", &v64) == 1) {
        return v64;
    }
    return 0;
}

void TextEditor::content(const std::wstring &value) {
    if (m_type == editor_integer) {
        uint32_t v64 = 0;
        if (swscanf(value.c_str(), L"%lld", &v64) != 1) {
            return;
        }
        wchar_t buffer[64] = L"";
        swprintf(buffer, sizeof(buffer)/2, L"%lld", v64);
        m_lines.clear();
        m_lines.push_back(buffer);
        return;
    };
    if (m_type == editor_float) {
        std::wstring tmp;
        for (size_t i = 0; i < value.size(); i++) {
            if (value[i] == u'.') { 
                if (tmp.find_first_of(u'.')) {
                    return;
                }
                if (tmp.empty() || tmp == L"-") {
                    tmp.push_back(u'0');
                }
                tmp.push_back(value[i]);
            } else if (value[i] < u'0' || value[i] > u'9') {
                return;
            } else {
                tmp.push_back(value[i]);
            }
        }

        m_lines.clear();
        m_lines.push_back(tmp);
        return;
    }

    std::wstringstream f(value);
    std::wstring tmp;
    m_lines.clear();
    while(std::getline<wchar_t>(f, tmp, U'\n')) {
        if (m_type == editor_multiline || m_type == editor_multiline_wrap) {
            m_lines.push_back(tmp + L"\n");
        } else {
            m_lines.push_back(tmp);
        }
        
        tmp.clear();
    }
    m_need_update = true;
}

sf::Text *TextEditor::text_display(size_t index) {
    auto font = load_default_font();
    if (!font) {
        return NULL;
    }
    while (m_texts.size() <= index) {
        m_texts.push_back(std::make_shared<sf::Text>(*font));
    }
    return m_texts[index].get();
}

void TextEditor::apply_format(sf::Text *txt) {
    if (txt) {
        int new_charsize = m_character_size * abs_scale();        
        if (txt->getCharacterSize() != new_charsize) {
            txt->setCharacterSize(new_charsize);
            txt->setPosition({0, 0});
        }
    }
}

bool TextEditor::update_measurement_item() {
    if (!m_text_measure) {
        auto fnt = load_default_font();
        if (!fnt) {
            return false;
        }
        m_text_measure.reset(new sf::Text(*fnt));
    }
    apply_format(m_text_measure.get());
    return true;
}

void TextEditor::update_scroll() {
    if (m_updating_scroll) return;
    m_updating_scroll = true;
    int disp_count = 0;
    int line_w, line_h;

    painting_metrics(line_w, line_h, disp_count);

    if (m_lines.size() < disp_count) {
        m_scroll_top = 0;
        m_vscrollbar->max(0);
        if (m_vertical_scrollbar == scrollbar_auto) {
            m_vscrollbar->visible(false);
        }
    } else {
        m_vscrollbar->max(m_lines.size() - disp_count);
        if (m_vertical_scrollbar == scrollbar_auto) {
            m_vscrollbar->visible(m_vscrollbar->max() > 0);
        }
    }

    if (m_scroll_top + disp_count <= m_cursor_y) {
        m_scroll_top = (m_cursor_y - disp_count) + 1;
    }
    if (m_cursor_y < m_scroll_top ) {
        m_scroll_top = m_cursor_y;
    }
    if (m_scroll_top > m_vscrollbar->max()) {
        m_scroll_top = m_vscrollbar->max();
    }
    m_vscrollbar->page_size(disp_count - 1);
    m_vscrollbar->value((int)m_scroll_top);
    m_updating_scroll = false;
}

void TextEditor::vertical_scrollbar_changed() {
    if (m_updating_scroll) return;
    int value = m_vscrollbar->value();
    int disp_count = 0;
    int line_w, line_h;
    painting_metrics(line_w, line_h, disp_count);
    int max_scroll = 0;
    if (m_lines.size() < disp_count) {
        max_scroll = 0;
    } else {
        max_scroll = m_lines.size() - disp_count;
    }
    if (value > max_scroll) {
        value = max_scroll;
    }
    m_scroll_top = value;
}

void TextEditor::wrap_text() {
    if (m_type != editor_type_t::editor_multiline_wrap && m_type != editor_type_t::editor_text_wrap) {
        return;
    }

    if (!m_need_update) {
        return;
    }

    if (!update_measurement_item()) {
        return;
    }

    auto unwrapped_cursor = unwrap_cursor();
    content(content());
    m_need_update = false;

    size_t i = 0;
    size_t limit_w = abs_w() - (theme::editor_margin() * 2) * abs_scale();
    sf::Vector2f location;
    bool should_continue = false;
    while (i < m_lines.size()) {
        should_continue = false;
        m_text_measure->setString(m_lines[i]);
        for (size_t char_pos = 0; char_pos < m_lines[i].size(); char_pos++) {
            location = m_text_measure->findCharacterPos(char_pos);
            if (location.x >= limit_w) {
                if (char_pos > 0) {
                    char_pos--;
                }
                for (auto i2 = char_pos; i2 > 0; i2--) {
                    if (m_lines[i][i2] == U' ' || m_lines[i][i2] == U'\t') {
                        char_pos = i2 + 1;
                        break;
                    }
                }
                should_continue = true;
                auto left = m_lines[i].substr(0, char_pos);
                if (char_pos > 0) {
                    m_text_measure->setString(left);
                    auto bounds = m_text_measure->getGlobalBounds();
                    if (bounds.size.x + bounds.position.x >= limit_w) {
                        char_pos--;
                        left = m_lines[i].substr(0, char_pos);
                    }
                }
                auto right = m_lines[i].substr(char_pos);
                m_lines[i] = left;
                i++;
                m_lines.insert(m_lines.begin() + i, right);
                break;
            }
        }
        if (!should_continue) {
            i++;
        }    
    }

    wrap_cursor(unwrapped_cursor);

    if (m_cursor_y < m_lines.size() && !m_lines[m_cursor_y].empty()) {
        if (m_cursor_x > 0 && m_lines[m_cursor_y][m_cursor_x - 1] == U'\n')  {
            m_cursor_x--;
        }
    }

    update_scroll();
}

wchar_t TextEditor::latest_character(size_t line_number) {
    if (m_lines[line_number].rbegin() != m_lines[line_number].rend()) {
        return *m_lines[line_number].rbegin();
    }
    return U'\0';
}

size_t TextEditor::unwrap_cursor(size_t x, size_t y) {
    size_t result = 0;
    size_t py = 0;
    while (py < m_lines.size()) {
        if (py == y) {
            result += x;
            break;
        }
        result += m_lines[py].size();
        py++;
    }
    return result; 
}

size_t TextEditor::unwrap_cursor() {
    return unwrap_cursor(m_cursor_x,  m_cursor_y);
}

void TextEditor::wrap_cursor(size_t value, size_t *x, size_t *y) {
    *x = 0;
    *y = 0;
    while (value > 0 && *y < m_lines.size()) {
        if (m_lines[*y].size() < value) {
            value -= m_lines[*y].size();
        } else {
            *x = value;
            value = 0;
            break;
        }
        (*y)++;
    }
}

void TextEditor::wrap_cursor(size_t value) {
    wrap_cursor(value, &m_cursor_x, &m_cursor_y);
}

void TextEditor::vertical_scrollbar(scrollbar_t value) {
    if (m_type != editor_multiline_wrap && m_type != editor_text_wrap && m_type != editor_multiline) {
        value = scrollbar_none;
    }
    m_vertical_scrollbar = value;
    if (value == scrollbar_none) {
        puts("Not visible vertical!");
        m_vscrollbar->visible(false);
    } else if (value == scrollbar_allways) {
        m_vscrollbar->visible(true);
    } else {
        update_scroll();
    }
    handle_parent_resized();
}

scrollbar_t TextEditor::vertical_scrollbar() {
    return m_vertical_scrollbar;
}

void TextEditor::horizontal_scrollbar(scrollbar_t value) {
    if (m_type == editor_multiline_wrap || m_type == editor_text_wrap) {
        value = scrollbar_none;
    }
    m_horizontal_scrollbar = value;
    if (value == scrollbar_none) {
        puts("Not visible horizontal!");
        m_hscrollbar->visible(false);
    } else if (value == scrollbar_allways) {
        m_hscrollbar->visible(true);
    } else {
        update_scroll();
    }
    handle_parent_resized();
}

scrollbar_t TextEditor::horizontal_scrollbar() {
    return m_horizontal_scrollbar;
}

void TextEditor::painting_metrics(int &line_width, int &line_height, int &disp_count ) {
    float scale = abs_scale();
    line_height = m_character_size + theme::editor_line_spacing();
    int hcoord = h() - (m_hscrollbar->visible() ? m_hscrollbar->h() : 0);
    disp_count = (m_character_size > 0 ? (hcoord / line_height) + 1 : 0) - 1;
    line_height = line_height * scale;
    line_width = (abs_w() - theme::editor_margin() * 2) * scale;
    line_width -= m_vscrollbar->visible() ? m_vscrollbar->abs_w() : 0;
}


void TextEditor::paint(sf::RenderTarget *render_target) {
    if (!m_vscrollbar->visible() && !m_hscrollbar->visible()) {
        paint_text_editor(render_target);
    } else {
        auto w =  m_vscrollbar->visible() ? m_vscrollbar->abs_w() : 0;
        auto h = m_hscrollbar->visible() ? m_hscrollbar->abs_h() : 0;
        paint_constraint(abs_x(), abs_y(), abs_w() - w, abs_h() - h, render_target->getSize().y, [this, render_target]() {
            this->paint_text_editor(render_target);
        });
    }
}

void TextEditor::paint_text_editor(sf::RenderTarget *render_target) {
    auto scale = abs_scale();
    wrap_text();

    // the component area
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);
    dw.color(m_fill_color);
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);

    int line_height = 0;
    int disp_count = 0;
    int line_width = 0;
    painting_metrics(line_width, line_height, disp_count);
    line_height = line_height * scale;
    int new_charsize = m_character_size * scale;
    int left_coord_ref = abs_x() + theme::editor_margin() * scale;
    int left_coord = left_coord_ref;
    int top_coord = abs_y();

    // the selection
    dw.size(line_width, line_height);
    dw.outline_color(m_selected_color);
    dw.color(m_selected_color);
    int line_number = 0;

    fix_selection();

    auto has_selection = this->has_selection();

    for (size_t i = 0; i < disp_count; i++) {
        left_coord = left_coord_ref;
        line_number = i + m_scroll_top;
        if (line_number >= m_lines.size()) {
            break;
        }
        const auto & current_content = m_lines[line_number];
        auto text_display = this->text_display(i);
        if (!text_display) break;
        
        if (new_charsize < 1) {
            new_charsize = 1;
        }
        if (text_display->getCharacterSize() != new_charsize) {
            text_display->setCharacterSize(new_charsize);
            // update_font_min_y_coord();
        }

        text_display->setFillColor(sf::Color(m_text_color));
        text_display->setOutlineColor(sf::Color(m_text_color));
        text_display->setString(m_type == editor_password ? std::wstring(current_content.size(), U'*') : current_content);

        if (m_halign == text_right) {
            left_coord = text_display->getLocalBounds().size.x;
            if (left_coord >= line_width) {
                left_coord = left_coord_ref;
            } else {
                left_coord = left_coord_ref + (line_width - left_coord);
            }
            text_display->setPosition({(float)left_coord, (float)top_coord});
        } else {
            text_display->setPosition({(float)left_coord, (float)top_coord});
        }

        if (!has_selection) {
            // does nothing
        } else if (line_number == m_selection_y1) {
            // first line of the selection
            if (m_selection_x1 < m_lines[line_number].size()) {

                auto p = text_display->findCharacterPos(m_selection_x1);

                if (m_selection_y1 == m_selection_y2) {

                    if (m_selection_x2 < m_lines[line_number].size()) {
                        auto p2 = text_display->findCharacterPos(m_selection_x2);
                        dw.size(p2.x - p.x, line_height);
                    } else {
                        if (m_lines[line_number].empty()) {
                            dw.size(0, line_height);
                        } else if (latest_character(line_number) == u'\n' && m_lines[line_number].size() > 1) {
                            auto p2 =  text_display->findCharacterPos(m_lines[line_number].size() - 2);
                            dw.size(p2.x, line_height);
                        } else {
                            auto p2 =  text_display->getGlobalBounds();
                            dw.size((p2.size.x + p2.position.x) - p.x, line_height);
                        }
                    }

                } else {
                    auto p2 = text_display->getGlobalBounds();
                    dw.size((p2.size.x + p2.position.x) - p.x, line_height);
                }
                dw.position(p.x, top_coord);    
                dw.draw(render_target);
            }
        } else if (line_number == m_selection_y2) {
            if (m_selection_x2 < m_lines[line_number].size()) {
                auto p = text_display->findCharacterPos(m_selection_x2);
                dw.size(p.x - left_coord, line_height);
                dw.position(left_coord, top_coord);    
                dw.draw(render_target);
            }
        } else if (line_number > m_selection_y1 && line_number < m_selection_y2) {
            if (!m_lines[line_number].empty()) {
                auto p = text_display->getGlobalBounds();
                dw.size(p.size.x, line_height);
                dw.position(left_coord, top_coord);
                dw.draw(render_target);
            }
        }

        top_coord += line_height;
        render_target->draw(*text_display);
        if (m_cursor_y == line_number && abs_enabled() && m_focused && clock::editor_cursor_visible())  {
            auto p = text_display->getPosition();
            p.x = text_display->findCharacterPos(m_cursor_x).x;
            sf::Vertex line[] = {
                {sf::Vector2f(p.x, p.y), sf::Color(m_cursor_color)},
                {sf::Vector2f(p.x, p.y + line_height), sf::Color(m_cursor_color)}
            };
            render_target->draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
    
    // the cursor
   
}

} // namespace dfe_ui
