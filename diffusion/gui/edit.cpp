#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "simple-ui/clock.h"
#include "simple-ui/edit.h"
#include "simple-ui/default_font.h"
#include "simple-ui/theme.h"

namespace dfe_ui
{

Edit::Edit(Window * window, int x, int y, int w, int h) : Component(window) {
    this->coordinates(x, y, w, h);
    auto font = load_default_font();
    if (font) {
        m_text.reset(new sf::Text(*font));
        update_font_min_y_coord();
    }
    cursor_color(dfe_ui::theme::editor_selection_color());
    selection_color(dfe_ui::theme::editor_selection_color());
    text_color(dfe_ui::theme::editor_text_color());
}

Edit::~Edit() {
}

void Edit::handle_focus_lost() {
    m_focused = false;
}

void Edit::handle_focus_got() {
    m_focused = true;
}

component_cursor_t Edit::cursor() {
    return cursor_edit;
}


void Edit::paint(sf::RenderTarget *render_target) {
    if (!m_text.get() || (m_password && m_line_wrap_enabled)) return;

    float scale = abs_scale();
    adjust_selection();

    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());

    int new_charsize = m_character_size * scale;
    if (new_charsize < 1) {
        new_charsize = 1;
    }
    if (txt.getCharacterSize() != new_charsize) {
        txt.setCharacterSize(new_charsize);
        update_font_min_y_coord();
    }

    txt.setFillColor(sf::Color(m_text_color));
    txt.setOutlineColor(sf::Color(m_text_color));

    if (m_last_scale != scale) {
        m_last_scale = scale;
        compute_wrap();
    }

    if (m_line_wrap_enabled && m_lines_offset.size() > 0) {
        const sf::String & s = txt.getString();
        sf::Text wrap_text(txt);
        int x = abs_x();
        int y = abs_y();
        for (size_t i = 1; i < m_lines_offset.size(); i++) {
            wrap_text.setString(s.substring(m_lines_offset[i - 1], m_lines_offset[i] - m_lines_offset[i - 1]));
            wrap_text.setPosition({x, y});
            y += wrap_text.getLocalBounds().size.y;
            render_target->draw(wrap_text);
        }
//        for ()
//        wrap_text.setString();
    } else {
        sf::Color targetColor = txt.getFillColor(); // (abs_enabled()) ? m_color : m_disabledColor;
        sf::CircleShape pw_shape;
        if ((!m_password) && txt.getFillColor() != targetColor)
            txt.setFillColor(targetColor);
        if (m_password && pw_shape.getFillColor() != targetColor)
            pw_shape.setFillColor(targetColor);

        int left = abs_x(), top = abs_y();
        int ttop = top - m_text_min_y;

        int char_sz = txt.getCharacterSize();

        if (m_text_valign == text_alignment_middle)
            ttop += abs_h() / 2 - char_sz / 2;
        else if (m_text_valign == text_alligment_bottom)
            ttop += abs_h() - char_sz;

        txt.setPosition({(float)left, (float)ttop});

        if (m_sel_start != m_sel_end) {
            int x1, x2;
            sf::RectangleShape sel_rect;
            sel_rect.setFillColor(sf::Color(m_selection_color));
            get_selection_area(x1, x2);
            sel_rect.setPosition({(float)x1, (float)top});
            sel_rect.setSize(sf::Vector2f(x2 - x1, abs_h()));
            render_target->draw(sel_rect);
        }

        if (m_password) {
            size_t l = txt.getString().getSize();
            unsigned int charSize = char_sz;
            float halfHeight = 0.5 * charSize;
            float dif = (abs_h() - charSize) * 0.5;
            charSize = halfHeight * 2.0;
            pw_shape.setRadius(halfHeight * 0.9);
            for (size_t i = 0; i < l; ++i) {
            pw_shape.setPosition(sf::Vector2f(i * charSize + abs_x(), abs_y() + dif ));
            render_target->draw(pw_shape);
            }
        } else {
            render_target->draw(txt);
        }
        if (abs_enabled() && m_focused && clock::editor_cursor_visible()) {
            int ipos = get_insert_coord();
            sf::Vertex line[] = {
                {sf::Vector2f(ipos, top), sf::Color(m_cursor_color)},
                {sf::Vector2f(ipos, top + abs_h()), sf::Color(m_cursor_color)}
            };
            render_target->draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

  

    //    txt.scale({abs_scale(), abs_scale()});
    // txt.setPosition({abs_x(), abs_y()});
    // ->draw(txt);
};

void Edit::get_selection_area(int& x1, int& x2) {
    if (!m_text.get()) return;
    adjust_selection();
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
    int char_sz = txt.getCharacterSize();

    if (m_password) {
        unsigned int charSize = char_sz;
        float halfHeight = 0.5 * charSize;
        charSize = halfHeight * 2.0;
        x1 = abs_x() + m_sel_start * charSize;
        x2 = abs_x() + m_sel_end * charSize;
        return;
    }

  sf::FloatRect r;
  if (m_sel_start > txt.getString().getSize() ||
     m_sel_end >= txt.getString().getSize() )
       r = txt.getGlobalBounds();

  if (m_sel_start <= txt.getString().getSize()) {
    sf::Vector2f v = txt.findCharacterPos(m_sel_start);
    x1 = v.x;
  } else
    x1 = r.position.x + r.size.x;

  if (m_sel_end == m_sel_start) {
    x2 = x1;
    return;
  }

  if (m_sel_end <= txt.getString().getSize()) {
    sf::Vector2f v = txt.findCharacterPos(m_sel_end);
    x2 = v.x;
  } else {
    x2 = r.position.x + r.size.x;
  }
}

int Edit::get_insert_coord() {
    if (!m_text.get()) return 0;
    adjust_selection();

    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());

   if (m_password) {
    unsigned int charSize = txt.getCharacterSize();
    float halfHeight = 0.5 * charSize;
    charSize = halfHeight * 2.0;
    return abs_x() + m_cursor_pos * charSize;
  }

  if (m_cursor_pos > txt.getString().getSize()) {
        sf::FloatRect r = txt.getGlobalBounds();
        return r.position.x + r.size.x;
  }

  if (m_cursor_pos <= txt.getString().getSize() && m_cursor_pos > 0) {
        sf::Vector2f v = txt.findCharacterPos(m_cursor_pos);
        return v.x;
  }

  return abs_x() + 1;
}

void Edit::text(const std::wstring& value) {
    if (!m_text) return;
    static_cast<sf::Text*>(m_text.get())->setString(value);
    compute_wrap();
}

std::wstring Edit::text() {
    if (!m_text) return std::wstring();
    return static_cast<sf::Text*>(m_text.get())->getString();
}

bool Edit::clickable() {
    return true;
}

bool Edit::editable() {
    return true;
}

bool Edit::focusable() {
    return true;
}

bool Edit::readonly() {
    return m_readonly;
}

void Edit::readonly(bool value) {
    m_readonly = value;
}

size_t Edit::maxlen() {
    return m_maxlen;
}

void Edit::maxlen(size_t value) {
    m_maxlen = value;
}

int Edit::character_size() {
    return m_character_size;
}

void Edit::character_size(int value) {
    m_character_size = value;
}

void Edit::handle_text_entered(wchar_t unicode) {
    if (m_readonly || !m_text.get()) return;

    char c[MB_CUR_MAX];
    int len = wctomb(c, unicode);
    if (len > 0) {
        unsigned char u = static_cast<unsigned char>(c[0]);
        if (u < 32 || u > 126) {
            return;
        }
    }

    clear_selection();
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());

    if ((m_maxlen > 0)&&(txt.getString().getSize() >= m_maxlen)) {
        return;
    }

    std::wstring data = txt.getString();

    if (m_cursor_pos < data.size())
        data.insert(m_cursor_pos, 1, unicode);
    else
        data.push_back(unicode);

    txt.setString(data);
    text_changed();

    ++m_cursor_pos;

    m_sel_start = m_cursor_pos;
    m_sel_end = m_cursor_pos;
}

void Edit::clear_selection() {
   if (m_readonly) return;
   
   if (m_sel_start != m_sel_end) {
      if (m_sel_start > m_sel_end) {
         size_t t = m_sel_end;
         m_sel_end = m_sel_start;
         m_sel_start = t;
      }

      sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
      std::wstring data = txt.getString();
      data.erase(m_sel_start, m_sel_end - m_sel_start);
      txt.setString(data);
      text_changed();
   }

   m_sel_end = m_sel_start;
   m_cursor_pos = m_sel_start;
}

void Edit::text_changed() {
    if (m_cb_text_changed) {
        m_cb_text_changed(this);
    }
}

void Edit::handle_keypressed(int key) {
    bool control_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RControl);
    bool shift_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RShift);

    switch ((sf::Keyboard::Key)key) {
        case sf::Keyboard::Key::A:
            if (control_pressed)
                select_all();
            break;
            case sf::Keyboard::Key::C:
            if (control_pressed)
                copy_to_clipboard();
            break;
            case sf::Keyboard::Key::V:
            if (control_pressed)
                past_from_clipboard();
            break;
            case sf::Keyboard::Key::X:
            if (control_pressed) {
                copy_to_clipboard();
                clear_selection();
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
                backspace_pressed(control_pressed);
            break;

            case sf::Keyboard::Key::Delete: delete_pressed(shift_pressed); break;
            case sf::Keyboard::Key::Left: left_pressed(shift_pressed); break;
            case sf::Keyboard::Key::Right: right_pressed(shift_pressed); break;
            case sf::Keyboard::Key::Home: home_pressed(shift_pressed); break;
            case sf::Keyboard::Key::End: end_pressed(shift_pressed); break;

            default:
            break;
    }
}

void Edit::left_pressed(bool shift) {
  adjust_selection();
  if (m_cursor_pos > 0) {
       if (m_sel_start == m_cursor_pos)
          --m_sel_start; else
       if (m_sel_end == m_cursor_pos)
          --m_sel_end;
       --m_cursor_pos;
  }
  if (!shift) {
    m_sel_end = m_cursor_pos;
    m_sel_start = m_cursor_pos;
  }
}

void Edit::right_pressed(bool shift) {
    adjust_selection();
    if (!m_text.get()) return;
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());

    if (m_cursor_pos < txt.getString().getSize()) {
        if (m_sel_start == m_cursor_pos)
            ++m_sel_start; else
        if (m_sel_end == m_cursor_pos)
            ++m_sel_end;
        ++m_cursor_pos;
    }

    if (!shift) {
        m_sel_end = m_cursor_pos;
        m_sel_start = m_cursor_pos;
    }
}

void Edit::home_pressed(bool shift) {
  adjust_selection();
  m_sel_start = 0;
  m_cursor_pos = 0;
  if (!shift)
    m_sel_end = m_sel_start;
}

void Edit::end_pressed(bool shift) {
    adjust_selection();
    if (!m_text.get()) return;
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
    m_sel_end = txt.getString().getSize();
    m_cursor_pos = m_sel_end;
    if (!shift) {
        m_sel_start = txt.getString().getSize();
        m_sel_end = m_sel_start;
    }
}

void Edit::adjust_selection() {
   if (m_sel_end >= m_sel_start) return;
   size_t tmp = m_sel_end;
   m_sel_end = m_sel_start;
   m_sel_start = tmp;
}

void Edit::backspace_pressed(bool control_pressed) {
    if (m_readonly || !m_text.get()) return;

    if (m_sel_start == m_sel_end) {
        if (m_sel_start <= 0) return;
            --m_sel_start;
    }

    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());

    if (control_pressed && m_sel_start > 0) {
        const sf::String& data = txt.getString();
        while(m_sel_start > 0){
            if ((m_sel_start < data.getSize()) && (data[m_sel_start] == 0x20)) {
                break;
            }
            --m_sel_start;
        }
    }

    clear_selection();
}


void Edit::select_all() {
    if (!m_text.get()) {
        return;
    }
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
    m_sel_start = 0;
    m_sel_end = txt.getString().getSize();
    m_cursor_pos = m_sel_end;
}


void Edit::delete_pressed(bool shift_pressed) {
    if (!m_text.get()) {
        return;
    }

    if (shift_pressed) {
        if (m_sel_start != m_sel_end) {
            copy_to_clipboard();
            clear_selection();
        } else {
            backspace_pressed(false);
        }
        return;
    }

    if (m_readonly) return;
    
    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
    if (m_sel_start == m_sel_end) {
        if (m_sel_end < txt.getString().getSize())
            ++m_sel_end;
    }

    clear_selection();
}

void Edit::update_font_min_y_coord() {
    if (m_text) {
        m_text_min_y = compute_text_min_y(m_text.get());
    }
}

void Edit::copy_to_clipboard() {
}

void Edit::past_from_clipboard() {
}

bool Edit::password() {
    return m_password;
}

void Edit::password(bool value) {
    m_password = value;
}

vertical_text_alignment_t Edit::text_valign() {
    return m_text_valign;
}

void Edit::text_valign(vertical_text_alignment_t value) {
    m_text_valign = value;
}

void Edit::handle_mouse_left_pressed(int x, int y) {
    m_mouse_down = true;
    m_mouse_down_x = x;
    handle_mouse_moved(x, y);
}

void Edit::handle_mouse_left_released(int x, int y) {
    m_mouse_down = false;
    m_mouse_down_x = x;
}

void Edit::handle_mouse_moved(int x, int y) {
    if (!m_mouse_down || !m_text) return;
    m_sel_start = get_character_pos_at_coord(x);
    m_sel_end = get_character_pos_at_coord(m_mouse_down_x);
    m_cursor_pos = m_sel_start;
    adjust_selection();
}

int Edit::get_character_pos_at_coord(int x) {
    if (!m_text) return 0;

    sf::Text &txt = *static_cast<sf::Text*>(m_text.get());
    x += txt.getPosition().x;
    if (m_password) {
        unsigned int charSize = character_size() * abs_scale();
        float halfHeight = 0.5 * charSize;
        charSize = halfHeight * 2.0;
        if (charSize) {
            unsigned int i =  (x / charSize);
            if (i > txt.getString().getSize())
                i = txt.getString().getSize();
            return i;
        }
        return 0;
    }

    size_t l = txt.getString().getSize();
    sf::Vector2f v;
    for (size_t i = 0; i < l; ++i) {
        v = txt.findCharacterPos(i);
        if (v.x >= x)
            return i;
    }

    return txt.getString().getSize();
}

void Edit::cursor_color(uint32_t color) {
    m_cursor_color = color;
}

void Edit::text_color(uint32_t color) {
    m_text_color = color;
}

void Edit::selection_color(uint32_t color) {
    m_selection_color = color;
}

uint32_t Edit::cursor_color() {
    return m_cursor_color;
}

uint32_t Edit::text_color() {
    return m_text_color;
}

uint32_t Edit::selection_color() {
    return m_selection_color;
}

bool Edit::line_wrap_enabled() {
    return m_line_wrap_enabled;
}

void Edit::compute_wrap() {
    if (!m_line_wrap_enabled || !m_text) return;
    m_lines_offset.clear();
    m_lines_offset.push_back(0);
    
    auto txt = static_cast<sf::Text *>(m_text.get());
    int current_width = 0;
    auto w = this->abs_w();
    size_t char_count = txt->getString().getSize();
    for (size_t i = 1; i < char_count; i++) {
        auto pos1 = txt->findCharacterPos(i - 1);
        auto pos2 = txt->findCharacterPos(i);
        int char_w = pos2.x - pos1.x;
        current_width += char_w;
        if (current_width > w) {
            m_lines_offset.push_back(i - 1);
            current_width = 0;
        }
    }
    if (*m_lines_offset.rbegin() != char_count) {
        m_lines_offset.push_back(char_count);
    }
}

void Edit::line_wrap_enabled(bool value) {
    m_line_wrap_enabled = value;
    compute_wrap();
}


} // namespace dfe_ui
