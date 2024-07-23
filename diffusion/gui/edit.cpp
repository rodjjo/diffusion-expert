#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "simple-ui/edit.h"
#include "simple-ui/default_font.h"

namespace dfe_ui
{

Edit::Edit(int x, int y, int w, int h) : Component() {
    this->coordinates(x, y, w, h);
    unsigned int bytes = 0;
    const void *font_buffer = load_default_font(bytes);
    if (auto fnt = sf::Font::openFromMemory(font_buffer, bytes)) {
        font_.reset(new sf::Font(*fnt), [] (void *f){ delete (sf::Font *) f;});
        text_.reset(new sf::Text(*(sf::Font *)font_.get()));
        update_font_min_y_coord();
    }
}

Edit::~Edit() {
}

void Edit::handle_focus_lost() {
    focused_ = false;
}

void Edit::handle_focus_got() {
    focused_ = true;
}


void Edit::paint(void *render_window) {
    if (!text_.get()) return;
    adjust_selection();

    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    int new_charsize = character_size_ * abs_scale();
    if (new_charsize < 1) {
        new_charsize = 1;
    }
    if (txt.getCharacterSize() != new_charsize) {
        txt.setCharacterSize(new_charsize * abs_scale());
    }
    
    auto wnd = static_cast<sf::RenderWindow *>(render_window);


    int64_t current_time = microseconds();
    
    bool blink = false;
    if (abs_enabled() && focused_){
        if (abs((last_blink_ + 120000) - current_time) < 120000) {
            blink = true;
        } else if (abs((last_blink_ + 240000) - current_time) > 240000) {
            last_blink_ = current_time;
        }
    }

    sf::Color targetColor = txt.getFillColor(); // (abs_enabled()) ? color_ : disabledColor_;
    sf::CircleShape pw_shape;
    if ((!password_) && txt.getFillColor() != targetColor)
        txt.setFillColor(targetColor);
    if (password_ && pw_shape.getFillColor() != targetColor)
        pw_shape.setFillColor(targetColor);


    int left = abs_x(), top = abs_y();
    int ttop = top - min_y_coord_;

    int char_sz = txt.getCharacterSize();

    // if (vertAlign_ == 1)
        ttop += abs_h() / 2 - char_sz / 2;
    // else if (vertAlign_ == 2)
    //    ttop += abs_h() - char_sz;

    txt.setPosition({left, ttop});

    if (sel_start_ != sel_end_) {
        int x1, x2;
        sf::RectangleShape sel_rect;
        sel_rect.setFillColor(sf::Color(64, 64, 64, 128));
        get_selection_area(x1, x2);
        sel_rect.setPosition({x1, top});
        sel_rect.setSize(sf::Vector2f(x2 - x1, abs_h()));
        wnd->draw(sel_rect);
    }

  if (password_) {
    size_t l = txt.getString().getSize();
    unsigned int charSize = char_sz;
    float halfHeight = 0.5 * charSize;
    float dif = (abs_h() - charSize) * 0.5;
    charSize = halfHeight * 2.0;
    pw_shape.setRadius(halfHeight * 0.9);
    for (size_t i = 0; i < l; ++i) {
      pw_shape.setPosition(sf::Vector2f(i * charSize + abs_x(), abs_y() + dif ));
      wnd->draw(pw_shape);
    }
  } else {
    wnd->draw(txt);
  }

  if (blink) {
    int ipos = get_insert_coord();
    sf::Vertex line[] = {
        {sf::Vector2f(ipos, top), sf::Color::Black},
        {sf::Vector2f(ipos, top + abs_h()), sf::Color::White}
    };
    wnd->draw(line, 2, sf::PrimitiveType::Lines);
  }

    //    txt.scale({abs_scale(), abs_scale()});
    // txt.setPosition({abs_x(), abs_y()});
    // ->draw(txt);
};

void Edit::get_selection_area(int& x1, int& x2) {
    if (!text_.get()) return;
    adjust_selection();
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    int char_sz = txt.getCharacterSize();

    if (password_) {
        unsigned int charSize = char_sz;
        float halfHeight = 0.5 * charSize;
        charSize = halfHeight * 2.0;
        x1 = abs_x() + sel_start_ * charSize;
        x2 = abs_x() + sel_end_ * charSize;
        return;
    }

  sf::FloatRect r;
  if (sel_start_ > txt.getString().getSize() ||
     sel_end_ >= txt.getString().getSize() )
       r = txt.getGlobalBounds();

  if (sel_start_ <= txt.getString().getSize()) {
    sf::Vector2f v = txt.findCharacterPos(sel_start_);
    x1 = v.x;
  } else
    x1 = r.position.x + r.size.x;

  if (sel_end_ == sel_start_) {
    x2 = x1;
    return;
  }

  if (sel_end_ <= txt.getString().getSize()) {
    sf::Vector2f v = txt.findCharacterPos(sel_end_);
    x2 = v.x;
  } else {
    x2 = r.position.x + r.size.x;
  }
}

int Edit::get_insert_coord() {
    if (!text_.get()) return 0;
    adjust_selection();

    sf::Text &txt = *static_cast<sf::Text*>(text_.get());

   if (password_) {
    unsigned int charSize = txt.getCharacterSize();
    float halfHeight = 0.5 * charSize;
    charSize = halfHeight * 2.0;
    return abs_x() + cursor_pos_ * charSize;
  }

  if (cursor_pos_ > txt.getString().getSize()) {
        sf::FloatRect r = txt.getGlobalBounds();
        return r.position.x + r.size.x;
  }

  if (cursor_pos_ <= txt.getString().getSize() && cursor_pos_ > 0) {
        sf::Vector2f v = txt.findCharacterPos(cursor_pos_);
        return v.x;
  }

  return abs_x() + 1;
}

void Edit::text(const std::wstring& value) {
    if (!text_) return;
    static_cast<sf::Text*>(text_.get())->setString(value);
}

std::wstring Edit::text() {
    if (!text_) return std::wstring();
    return static_cast<sf::Text*>(text_.get())->getString();
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
    return readonly_;
}

void Edit::readonly(bool value) {
    readonly_ = value;
}

size_t Edit::maxlen() {
    return maxlen_;
}

void Edit::maxlen(size_t value) {
    maxlen_ = value;
}

int Edit::character_size() {
    return character_size_;
}

void Edit::character_size(int value) {
    character_size_ = value;
}

void Edit::handle_textentered(wchar_t unicode) {
    if (readonly_ || !text_.get()) return;

    char c[MB_CUR_MAX];
    int len = wctomb(c, unicode);
    if (len < 1) return;
    unsigned char u = static_cast<unsigned char>(c[0]);
    if (u < 32 /*|| u > 126*/) {
        return;
    }

    clear_selection();
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());


    if ((maxlen_ > 0)&&(txt.getString().getSize() >= maxlen_)) {
        return;
    }

    std::wstring data = txt.getString();

    if (cursor_pos_ < data.size())
        data.insert(cursor_pos_, 1, unicode);
    else
        data.push_back(unicode);

    txt.setString(data);
    text_changed();

    ++cursor_pos_;

    sel_start_ = cursor_pos_;
    sel_end_ = cursor_pos_;
}

void Edit::clear_selection() {
   if (readonly_) return;
   
   if (sel_start_ != sel_end_) {
      if (sel_start_ > sel_end_) {
         size_t t = sel_end_;
         sel_end_ = sel_start_;
         sel_start_ = t;
      }

      sf::Text &txt = *static_cast<sf::Text*>(text_.get());
      std::string data = txt.getString();
      data.erase(sel_start_, sel_end_ - sel_start_);
      txt.setString(data);
      text_changed();
   }

   sel_end_ = sel_start_;
   cursor_pos_ = sel_start_;
}

void Edit::text_changed() {
    if (cb_text_changed_) {
        cb_text_changed_(this);
    }
}

void Edit::handle_keypressed(int key) {
    last_blink_ = 0;
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
                //if (listener_)
                // listener_->fireEditSpecKeyPressed(ev);
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
  if (cursor_pos_ > 0) {
       if (sel_start_ == cursor_pos_)
          --sel_start_; else
       if (sel_end_ == cursor_pos_)
          --sel_end_;
       --cursor_pos_;
  }
  if (!shift) {
    sel_end_ = cursor_pos_;
    sel_start_ = cursor_pos_;
  }
}

void Edit::right_pressed(bool shift) {
    adjust_selection();
    if (!text_.get()) return;
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());

    if (cursor_pos_ < txt.getString().getSize()) {
        if (sel_start_ == cursor_pos_)
            ++sel_start_; else
        if (sel_end_ == cursor_pos_)
            ++sel_end_;
        ++cursor_pos_;
    }

    if (!shift) {
        sel_end_ = cursor_pos_;
        sel_start_ = cursor_pos_;
    }
}

void Edit::home_pressed(bool shift) {
  adjust_selection();
  sel_start_ = 0;
  cursor_pos_ = 0;
  if (!shift)
    sel_end_ = sel_start_;
}

void Edit::end_pressed(bool shift) {
    adjust_selection();
    if (!text_.get()) return;
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    sel_end_ = txt.getString().getSize();
    cursor_pos_ = sel_end_;
    if (!shift) {
        sel_start_ = txt.getString().getSize();
        sel_end_ = sel_start_;
    }
}

void Edit::adjust_selection() {
   if (sel_end_ >= sel_start_) return;
   size_t tmp = sel_end_;
   sel_end_ = sel_start_;
   sel_start_ = tmp;
}

void Edit::backspace_pressed(bool control_pressed) {
    if (readonly_ || !text_.get()) return;

    if (sel_start_ == sel_end_) {
        if (sel_start_ <= 0) return;
            --sel_start_;
    }

    sf::Text &txt = *static_cast<sf::Text*>(text_.get());

    if (control_pressed && sel_start_ > 0) {
        const sf::String& data = txt.getString();
        while(sel_start_ > 0){
            if ((sel_start_ < data.getSize()) && (data[sel_start_] == 0x20)) {
                break;
            }
            --sel_start_;
        }
    }

    clear_selection();
}


void Edit::select_all() {
    if (!text_.get()) {
        return;
    }
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    sel_start_ = 0;
    sel_end_ = txt.getString().getSize();
    cursor_pos_ = sel_end_;
}


void Edit::delete_pressed(bool shift_pressed) {
    if (!text_.get()) {
        return;
    }

    if (shift_pressed) {
        if (sel_start_ != sel_end_) {
            copy_to_clipboard();
            clear_selection();
        } else {
            backspace_pressed(false);
        }
        return;
    }

    if (readonly_) return;
    
    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    if (sel_start_ == sel_end_) {
        if (sel_end_ < txt.getString().getSize())
            ++sel_end_;
    }

    clear_selection();
}

void Edit::update_font_min_y_coord() {
    if (text_) {
        min_y_coord_ = compute_text_min_y(text_.get());
    }
}

void Edit::copy_to_clipboard() {
}

void Edit::past_from_clipboard() {
}

bool Edit::password() {
    return password_;
}

void Edit::password(bool value) {
    password_ = value;
}

void Edit::handle_mouse_left_pressed(int x, int y) {
    mouse_down_ = true;
    mouse_down_x_ = x;
    handle_mouse_moved(x, y);
}

void Edit::handle_mouse_left_released(int x, int y) {
    mouse_down_ = false;
    mouse_down_x_ = x;
}

void Edit::handle_mouse_moved(int x, int y) {
    if (!mouse_down_ || !text_) return;
    sel_start_ = get_character_pos_at_coord(x);
    sel_end_ = get_character_pos_at_coord(mouse_down_x_);
    cursor_pos_ = sel_start_;
    adjust_selection();
}

int Edit::get_character_pos_at_coord(int x) {
    if (!text_) return 0;

    sf::Text &txt = *static_cast<sf::Text*>(text_.get());
    x += txt.getPosition().x;
    if (password_) {
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
    cursor_color_ = color;
}

void Edit::text_color(uint32_t color) {
    text_color_ = color;
}

void Edit::selection_color(uint32_t color) {
    selection_color_ = color;
}

uint32_t Edit::cursor_color() {
    return cursor_color_;
}

uint32_t Edit::text_color() {
    return text_color_;
}

uint32_t Edit::selection_color() {
    return selection_color_;
}


} // namespace dfe_ui
