#include <sstream>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Graphics/Text.hpp>

#include "simple-ui/theme.h"
#include "simple-ui/text_editor.h"

#include "drawings.h"

namespace dfe_ui
{

TextEditor::TextEditor(Window * window, int x, int y, int w, int h, editor_type_t type) : Component(window) {
    text_color(dfe_ui::theme::editor_text_color());
    cursor_color(dfe_ui::theme::editor_cursor_color());
    selected_color(dfe_ui::theme::editor_selection_color());
    outline_color(dfe_ui::theme::editor_outline_color());
    fill_color(dfe_ui::theme::editor_fill_color());
    this->coordinates(x, y, w, h);
}

void TextEditor::paint(sf::RenderTarget *render_target) {
    Drawing dw(Drawing::drawing_flat_box);
    dw.outline_color(m_outline_color);
    dw.color(m_fill_color);
    dw.size(abs_w(), abs_h());
    dw.position(abs_x(), abs_y());
    dw.margin(0);
    dw.draw(render_target);
    
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

editor_vertical_aligment_t TextEditor::valign() {
    return m_valign;
}

void TextEditor::halign(editor_vertical_aligment_t value) {
    m_valign = value;
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

}

void TextEditor::handle_textentered(wchar_t unicode) {
    if (m_readonly) return;

    char c[MB_CUR_MAX];
    int len = wctomb(c, unicode);
    if (len > 0) {
        unsigned char u = static_cast<unsigned char>(c[0]);
        if (u < 32 || u > 126) {
            return;
        }
    }

    remove_selected_text();

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
            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::Enter:
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

}

void TextEditor::left_pressed() {
    if (m_cursor_x > 0) {
        m_cursor_x--;
    }
}

void TextEditor::right_pressed() {
    
}

void TextEditor::home_pressed() {
    m_cursor_x = 0;
    if (is_control_pressed()) {
        m_cursor_y = 0;
    }
}

void TextEditor::end_pressed() {
    if (is_control_pressed()) {
        m_cursor_y = m_lines.size() - 1;
    }
    if (m_cursor_y < m_lines.size()) {
        m_cursor_x = m_lines[m_cursor_y].size();
    } else {
        m_cursor_x = 0;
    }
}

void TextEditor::select_nothing() {
    m_selection_x1 = 0;
    m_selection_y1 = 0;
    m_selection_x2 = 0;
    m_selection_y2 = 0;
}

void TextEditor::select_all() {
    select_nothing();
    if (m_lines.size()) {
        m_selection_y2 = m_lines.size() - 1;
        m_selection_x2 = m_lines[m_selection_y2].size();
    }
}

void TextEditor::fix_selection() {
    if (m_lines.empty()) {
        select_nothing();
        return;
    }

    if (m_selection_y1 >= m_lines.size()) {
        m_selection_y1 = m_lines.size() - 1;
        m_selection_x1  = m_lines[m_selection_y1].size();
    }

    if (m_selection_y2 >= m_lines.size()) {
        m_selection_y2 = m_lines.size() - 1;
        m_selection_x2  = m_lines[m_selection_y2].size();
    }

    if (m_selection_y1 > m_selection_y2) {
        auto tmp = m_selection_y1;
        m_selection_y1 = m_selection_y2;
        m_selection_y2 = tmp;
        tmp = m_selection_x1;
        m_selection_x1 = m_selection_x2;
        m_selection_x2 = tmp;
    }
    if (m_selection_y1 == m_selection_y2) {
        if (m_selection_x2 > m_selection_x1) {
            auto tmp = m_selection_x2;
            m_selection_x2 = m_selection_x1;
            m_selection_x1 = tmp;
        }
    }
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
    sf::Clipboard::setString(get_selected_text());
}

void TextEditor::paste_from_clipboard() {
    auto value = sf::Clipboard::getString();
    if (!value.isEmpty()) {
        
    }
}

void TextEditor::delete_pressed() {

}

void TextEditor::handle_focus_lost() {

}

void TextEditor::handle_focus_got() {

}

void TextEditor::handle_mouse_left_pressed(int x, int y) {

}

void TextEditor::handle_mouse_left_released(int x, int y) {

}

void TextEditor::handle_mouse_moved(int x, int y) {

}

std::wstring TextEditor::content() {
    std::wstring result;
    for (auto & l : m_lines) {
        result += l;
    }
    return result;
}

void TextEditor::content(const std::wstring &value) {
    std::wstringstream f(value);
    std::wstring tmp;
    m_lines.clear();
    while(std::getline<wchar_t>(f, tmp, U'\n')) {
        m_lines.push_back(tmp);
        tmp.clear();
    }
}


} // namespace dfe_ui
