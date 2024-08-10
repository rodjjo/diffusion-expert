#include <SFML/Graphics.hpp>

#include "simple-ui/clock.h"
#include "simple-ui/icons.h"
#include "simple-ui/win.h"

namespace dfe_ui {


Window::Window(unsigned int w, unsigned int h, const char *title) : Component(NULL) {
    m_window.reset(
        new sf::RenderWindow(sf::VideoMode({w, h}),
                            title,
                            sf::Style::Titlebar | sf::Style::Close,
                            sf::State::Windowed,
                            sf::ContextSettings{0 /* depthBits */, 8 /* stencilBits */})
    );
    int x = 0;
    int y = 0;
    m_window->setPosition(sf::Vector2i(x, y));
    load_icons_texture();
    coordinates(x, y, w, h);
}

Window::~Window() {
    unload_icons_texture();
}

void Window::run() {
    auto window = m_window.get();
    
    clock::restart_clock();

    while (window->isOpen())
    {
        // Handle events
        while (const std::optional event = window->pollEvent())
        {
            // Window closed: exit
            if (event->is<sf::Event::Closed>())
            {
                window->close();
                break;
            }
            
            if (const auto* const resized_event = event->getIf<sf::Event::Resized>()) {
                window->setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(resized_event->size.x, resized_event->size.y))));
                this->size(resized_event->size.x, resized_event->size.y);
            } else if (const auto* const pressed_event = event->getIf<sf::Event::TextEntered>()) {
                this->handle_text_entered(pressed_event->unicode);
            } else if (const auto* const mouse_pressed_event = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse_pressed_event->button == sf::Mouse::Button::Left) {
                    this->m_mouse_left_pressed = true;
                    this->handle_mouse_left_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } else if (mouse_pressed_event->button == sf::Mouse::Button::Middle) {
                    this->m_mouse_middle_pressed = true;
                    this->handle_mouse_middle_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } else if (mouse_pressed_event->button == sf::Mouse::Button::Right) {
                    this->m_mouse_middle_pressed = true;
                    this->handle_mouse_right_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } 
            } else if (const auto* const mouse_released_event = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouse_released_event->button == sf::Mouse::Button::Left) {
                    this->m_mouse_left_pressed = false;
                    this->handle_mouse_left_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } else if (mouse_released_event->button == sf::Mouse::Button::Middle) {
                    this->m_mouse_middle_pressed = false;
                    this->handle_mouse_middle_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } else if (mouse_released_event->button == sf::Mouse::Button::Right) {
                    this->m_mouse_middle_pressed = false;
                    this->handle_mouse_right_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } 
            } else if (const auto* const mouse_moved_event = event->getIf<sf::Event::MouseMoved>()) {
                this->handle_mouse_moved(mouse_moved_event->position.x, mouse_moved_event->position.y);
                if (m_mouse_left_pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    m_mouse_left_pressed = false;
                    this->handle_mouse_left_released(mouse_moved_event->position.x, mouse_moved_event->position.y);
                }
                if (m_mouse_right_pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                    m_mouse_right_pressed = false;
                    this->handle_mouse_right_released(mouse_moved_event->position.x, mouse_moved_event->position.y);
                }
                if (m_mouse_middle_pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                    m_mouse_middle_pressed = false;
                    this->handle_mouse_middle_released(mouse_moved_event->position.x, mouse_moved_event->position.y);
                }
                update_cursor(window);
            } else if (event->is<sf::Event::MouseEntered>()) {
                /*
                    mouse enters the window
                */
            } else if (event->is<sf::Event::MouseLeft>()) {
                /*
                    mouse exits the window
                */
            } else if (const auto* const keypressed_event = event->getIf<sf::Event::KeyPressed>()) {
                this->handle_keypressed(static_cast<int>(keypressed_event->code));
            } else if (const auto* const wheel_event = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel_event->delta != 0) {
                    this->handle_mouse_wheel(wheel_event->delta > 0 ? 1 : -1, wheel_event->position.x, wheel_event->position.y);
                }
            }             
        }
        window->clear(sf::Color::Black, 0);
        paint_children(window, true);
        for (auto it = m_floating_components.rbegin(); it != m_floating_components.rend(); it++) {
            it->get()->paint_children(window);
        }
        if (m_component_in_drag) {
            m_component_in_drag->paint_children(window, false);
        }
        window->display();
    }
}

void Window::handle_parent_resized() {
    // this windows has no parent, so it was resized instead.
}

void Window::handle_text_entered(wchar_t unicode) {
    if (this->m_component_in_focus) {
        this->m_component_in_focus->handle_text_entered(unicode);
    }
}

void Window::replace_focus(Component *component) {
    if (!component->focusable()) {
        return;
    }
    if (m_component_in_focus.get() != NULL && component != m_component_in_focus.get()) {
        auto lost = m_component_in_focus;
        m_component_in_focus = component->share();
        lost->handle_focus_lost();
        m_component_in_focus->handle_focus_got();
    } else if (!m_component_in_focus) {
        m_component_in_focus = component->share();
        m_component_in_focus->handle_focus_got();
    }
}

void Window::remove_focus() {
    if (m_component_in_focus) {
        auto lost = m_component_in_focus;
        m_component_in_focus.reset();
        lost->handle_focus_lost();
    }
}

void Window::handle_mouse_left_pressed(int x, int y) {
    m_mouse_down_x = x;
    m_mouse_down_y = y;
    m_mouse_move_x = x;
    m_mouse_move_y = y;
    auto component = component_at_mouse(x, y);
    if (component) {
        replace_focus(component);
        component->handle_mouse_left_pressed(x, y);
        replace_drag(component);
        update_drag_coord();
        m_component_in_mouse_down_left = component->share();
    } else {
        remove_drag();
        m_component_in_mouse_down_left.reset();
    }
}

void Window::handle_mouse_middle_pressed(int x, int y) {
    auto component = component_at_mouse(x, y);
    if (component) {
        replace_focus(component);
        component->handle_mouse_middle_pressed(x, y);
        m_component_in_mouse_down_middle = component->share();
    }
}

void Window::handle_mouse_right_pressed(int x, int y) {
    auto component = component_at_mouse(x, y);
    if (component) {
        replace_focus(component);
        component->handle_mouse_right_pressed(x, y);
        m_component_in_mouse_down_right = component->share();
    }
}

void Window::replace_drag(Component *component) {
    if (!component->drag_enabled() || component == m_component_in_drag.get()) {
        return;
    }
    auto lost = m_component_in_drag;
    m_component_in_drag = component->share();
    if (lost) {
        lost->drag_end();
        drop_end(lost.get());
    }
    m_component_in_drag->drag_begin();
    drop_begin(m_component_in_drag.get());
}

void Window::remove_drag() {
    if (m_component_in_drag) {
        m_component_in_drag->drag_end();
        drop_end(m_component_in_drag.get());
        m_component_in_drag.reset();
    }
}

void Window::complete_drag(Component *component) {
    if (!m_component_in_drag || component == m_component_in_drag->parent()) {
        return;
    }
    if (component->accept_drop(m_component_in_drag.get()) && m_component_in_drag->accept_drag(component) ) {
        component->complete_drop(m_component_in_drag.get());
    }
}

void Window::fix_mouse_coords(int &x, int &y) {
    if (x >= w()) {
        x = w() - 1;
    }
    if (y >= h()) {
        y = h() - 1;
    }
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
}

void Window::handle_mouse_left_released(int x, int y) {
    fix_mouse_coords(x, y);
    auto released_ptr = m_component_in_mouse_down_left.get();
    if (m_component_in_mouse_down_left) {
        m_component_in_mouse_down_left->handle_mouse_left_released(x - m_component_in_mouse_down_left->abs_x(), y - m_component_in_mouse_down_left->abs_y());
        m_component_in_mouse_down_left.reset();
    }
    
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        if (released_ptr != released_ptr) {
            component->handle_mouse_left_released(x, y);
        } else {
            component->handle_click();
        }
        complete_drag(component);
    }
    remove_drag();
}


void Window::handle_mouse_middle_released(int x, int y) {
    fix_mouse_coords(x, y);
    auto released_ptr = m_component_in_mouse_down_middle.get();
    if (m_component_in_mouse_down_middle) {
        m_component_in_mouse_down_middle->handle_mouse_middle_released(x - m_component_in_mouse_down_middle->abs_x(), y - m_component_in_mouse_down_middle->abs_y());
        m_component_in_mouse_down_middle.reset();
    }
    
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        if (component != released_ptr) {
            component->handle_mouse_middle_released(x, y);
        }
    }
}

void Window::handle_mouse_right_released(int x, int y) {
    fix_mouse_coords(x, y);
    auto released_ptr = m_component_in_mouse_down_right.get();
    if (m_component_in_mouse_down_right) {
        m_component_in_mouse_down_right->handle_mouse_middle_released(x - m_component_in_mouse_down_right->abs_x(), y - m_component_in_mouse_down_right->abs_y());
        m_component_in_mouse_down_right.reset();
    }
    
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        if (component != released_ptr) {
            component->handle_mouse_right_released(x, y);
        }
    }
}

void Window::handle_mouse_moved(int x, int y) {
    if (m_component_in_mouse_down_right) {
        // m_component_in_mouse_down_right->handle_mouse_moved(x - m_component_in_mouse_down_right->abs_x(), y - m_component_in_mouse_down_right->abs_y());
    }
    if (m_component_in_mouse_down_left) {
        float s = m_component_in_mouse_down_left->abs_scale();
        if (s != 0) {
            int xcoord = x / s - m_component_in_mouse_down_left->abs_x() / s;
            int ycoord = y / s - m_component_in_mouse_down_left->abs_y() / s;
            // xcoord = xcoord / s;
            // ycoord = ycoord / s;
            m_component_in_mouse_down_left->handle_mouse_moved(xcoord, ycoord);
        }
    }
    if (m_component_in_mouse_down_middle) {
        // m_component_in_mouse_down_middle->handle_mouse_moved(x - m_component_in_mouse_down_middle->abs_x(), y - m_component_in_mouse_down_middle->abs_y());
    }
    m_mouse_move_x = x;
    m_mouse_move_y = y;
    auto component = component_at_mouse(x, y);
    if (component) {
        component->handle_mouse_moved(x, y);
        replace_mouse(component);
    } else {
        remove_mouse();
    }
    update_drag_coord();
}

void Window::update_drag_coord() {
    if (!m_component_in_drag) {
        return;
    }
    int coord_x = (m_mouse_move_x - m_mouse_down_x);
    int coord_y = (m_mouse_move_y - m_mouse_down_y);
    m_component_in_drag->set_drag_coord(coord_x, coord_y);
}

void Window::handle_keypressed(int key) {
    if (m_component_in_focus) {
        m_component_in_focus->handle_keypressed(key);
    }
}

void Window::handle_mouse_wheel(int8_t direction, int x, int y) {
    auto component = component_at_mouse(x, y);
    if (component) {
        component->handle_mouse_wheel(direction, x, y);
    }
}

void Window::replace_mouse(Component *component) {
    if (m_component_in_mouse.get() == component) {
        return;
    }
    if (m_component_in_mouse) {
        m_component_in_mouse->mouse_exit();
    }
    m_component_in_mouse = component->share();
    m_component_in_mouse->mouse_enter();
}

void Window::remove_mouse() {
    if (m_component_in_mouse) {
        m_component_in_mouse->mouse_exit();
        m_component_in_mouse.reset();
    }
}

void Window::update_cursor(sf::RenderTarget *render_target) {
    static auto cur_arrow = sf::Cursor::loadFromSystem(sf::Cursor::Type::Arrow);
    static auto cur_hand = sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand);
    static auto cur_sizeall = sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeAll);
    static auto cur_edit = sf::Cursor::loadFromSystem(sf::Cursor::Type::Text);
    
    auto wnd = static_cast<sf::RenderWindow *>(render_target);
    sf::Cursor::Type cursor_type = sf::Cursor::Type::Arrow;
    if (cur_arrow && cur_hand && cur_sizeall && cur_edit) {
        if (m_component_in_drag) {
            wnd->setMouseCursor(*cur_sizeall);
        } else if (m_component_in_mouse) {
            if (m_component_in_mouse->cursor() == cursor_drag) {
                wnd->setMouseCursor(*cur_sizeall);
            } else if (m_component_in_mouse->cursor() == cursor_hand) {
                wnd->setMouseCursor(*cur_hand);
            } else if (m_component_in_mouse->cursor() == cursor_edit) {
                wnd->setMouseCursor(*cur_edit);
            } else {
                wnd->setMouseCursor(*cur_arrow);
            }
        } else {
            wnd->setMouseCursor(*cur_arrow);
        }
    }
}

Component *Window::component_at_mouse(int &x, int &y, Component **floatting) {
    if (floatting) {
        *floatting = NULL;
    }

    int x_save = x;
    int y_save = y;
    for (auto & i : m_floating_components) {
        if (auto c = i->find_top_clickable(x, y)) {
            if (floatting) {
                *floatting = i.get();
            }
            return c;
        }
        x = x_save;
        y = y_save;
    }

    x = x_save;
    y = y_save;
    return find_top_clickable(x, y);
}

Component *Window::component_at_mouse(int &x, int &y) {
    return component_at_mouse(x, y, NULL);
}


void Window::add_floating_commponent(Component *comp) {
    if (comp == this) return;
    for (auto & i : m_floating_components) {
        if (comp == i.get()) {
            return;
        }
    }
    m_floating_components.push_front(comp->share());
}

void Window::remove_floating_commponent(Component *comp) {
    if (comp == this) return;
    for (auto it = m_floating_components.begin(); it != m_floating_components.end(); it++) {
        if (it->get() == comp) {
            it->get()->handle_float_off();
            m_floating_components.erase(it);
            return;
        }
    }
}

bool Window::is_floatting_component(Component *comp) {
    if (comp == this) return false;
    for (auto it = m_floating_components.begin(); it != m_floating_components.end(); it++) {
        if (it->get() == comp) {
            return true;
        }
    }
    return false;
}

void Window::pop_front_floatting_components(Component *floatting) {
    while(m_floating_components.begin() != m_floating_components.end()) {
        if (m_floating_components.begin()->get() == floatting) {
            return;
        }
        m_floating_components.begin()->get()->handle_float_off();
        m_floating_components.erase(m_floating_components.begin());
    }
}


std::shared_ptr<Window> window_new(int w, int h, const char *title) {
    return std::make_shared<Window>(Window(w, h, title));
}



} // namespace dfe
