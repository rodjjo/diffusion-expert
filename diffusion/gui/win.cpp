#include <SFML/Graphics.hpp>

#include "simple-ui/clock.h"
#include "simple-ui/icons.h"
#include "simple-ui/win.h"

namespace dfe_ui {


Window::Window(unsigned int w, unsigned int h, const char *title) : Component(this) {
    window_.reset(
        new sf::RenderWindow(sf::VideoMode({w, h}),
                            title,
                            sf::Style::Titlebar | sf::Style::Close,
                            sf::State::Windowed,
                            sf::ContextSettings{0 /* depthBits */, 8 /* stencilBits */})
    , [](void *p) { delete static_cast<sf::RenderWindow*>(p);});
    int x = 0;
    int y = 0;
    static_cast<sf::RenderWindow*>(window_.get())->setPosition(sf::Vector2i(x, y));
    load_icons_texture();
}

Window::~Window() {
    unload_icons_texture();
}

void Window::run() {
    auto window = static_cast<sf::RenderWindow*>(window_.get());
    
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
                this->handle_textentered(pressed_event->unicode);
            } else if (const auto* const mouse_pressed_event = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse_pressed_event->button == sf::Mouse::Button::Left) {
                    this->mouse_left_pressed_ = true;
                    this->handle_mouse_left_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } else if (mouse_pressed_event->button == sf::Mouse::Button::Middle) {
                    this->mouse_middle_pressed_ = true;
                    this->handle_mouse_middle_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } else if (mouse_pressed_event->button == sf::Mouse::Button::Right) {
                    this->mouse_middle_pressed_ = true;
                    this->handle_mouse_right_pressed(mouse_pressed_event->position.x, mouse_pressed_event->position.y);
                } 
            } else if (const auto* const mouse_released_event = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouse_released_event->button == sf::Mouse::Button::Left) {
                    this->mouse_left_pressed_ = false;
                    this->handle_mouse_left_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } else if (mouse_released_event->button == sf::Mouse::Button::Middle) {
                    this->mouse_middle_pressed_ = false;
                    this->handle_mouse_middle_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } else if (mouse_released_event->button == sf::Mouse::Button::Right) {
                    this->mouse_middle_pressed_ = false;
                    this->handle_mouse_right_released(mouse_released_event->position.x, mouse_released_event->position.y);
                } 
            } else if (const auto* const mouse_moved_event = event->getIf<sf::Event::MouseMoved>()) {
                this->handle_mouse_moved(mouse_moved_event->position.x, mouse_moved_event->position.y);
                if (mouse_left_pressed_ && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    mouse_left_pressed_ = false;
                    this->handle_mouse_left_released(mouse_moved_event->position.x, mouse_moved_event->position.y);
                }
                if (mouse_right_pressed_ && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                    mouse_right_pressed_ = false;
                    this->handle_mouse_right_released(mouse_moved_event->position.x, mouse_moved_event->position.y);
                }
                if (mouse_middle_pressed_ && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                    mouse_middle_pressed_ = false;
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
        for (auto it = floating_components_.rbegin(); it != floating_components_.rend(); it++) {
            it->get()->paint_children(window);
        }
        if (component_in_drag_) {
            component_in_drag_->paint_children(window, false);
        }
        window->display();
    }
}

void Window::handle_parent_resized() {
    // this windows has no parent, so it was resized instead.
}

void Window::handle_textentered(wchar_t unicode) {
    if (this->component_in_focus_) {
        this->component_in_focus_->handle_textentered(unicode);
    }
}

void Window::replace_focus(Component *component) {
    if (!component->focusable()) {
        return;
    }
    if (component_in_focus_.get() != NULL && component != component_in_focus_.get()) {
        auto lost = component_in_focus_;
        component_in_focus_ = component->share();
        lost->handle_focus_lost();
        component_in_focus_->handle_focus_got();
    } else if (!component_in_focus_) {
        component_in_focus_ = component->share();
        component_in_focus_->handle_focus_got();
    }
}

void Window::remove_focus() {
    if (component_in_focus_) {
        auto lost = component_in_focus_;
        component_in_focus_.reset();
        lost->handle_focus_lost();
    }
}

void Window::handle_mouse_left_pressed(int x, int y) {
    mouse_down_x_ = x;
    mouse_down_y_ = y;
    mouse_move_x_ = x;
    mouse_move_y_ = y;
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        replace_focus(component);
        component->handle_mouse_left_pressed(x, y);
        replace_drag(component);
        update_drag_coord();
        component_in_mouse_down_left_ = component->share();
    } else {
        remove_drag();
    }
}

void Window::handle_mouse_middle_pressed(int x, int y) {
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        replace_focus(component);
        component->handle_mouse_middle_pressed(x, y);
        component_in_mouse_down_middle_ = component->share();
    }
}

void Window::handle_mouse_right_pressed(int x, int y) {
    Component *floatting = NULL;
    auto component = component_at_mouse(x, y, &floatting);
    pop_front_floatting_components(floatting);

    if (component) {
        replace_focus(component);
        component->handle_mouse_right_pressed(x, y);
        component_in_mouse_down_right_ = component->share();
    }
}

void Window::handle_mouse_left_released(int x, int y) {
    auto released_ptr = component_in_mouse_down_left_.get();
    if (component_in_mouse_down_left_) {
        component_in_mouse_down_left_->handle_mouse_left_released(x - component_in_mouse_down_left_->abs_x(), y - component_in_mouse_down_left_->abs_y());
        component_in_mouse_down_left_.reset();
    }
    auto component = component_at_mouse(x, y);
    if (component) {
        if (released_ptr != released_ptr) {
            component->handle_mouse_left_released(x, y);
        }
        complete_drag(component);
    }
    remove_drag();
}

void Window::replace_drag(Component *component) {
    if (!component->drag_enabled() || component == component_in_drag_.get()) {
        return;
    }
    auto lost = component_in_drag_;
    component_in_drag_ = component->share();
    if (lost) {
        lost->drag_end();
        drop_end(lost.get());
    }
    component_in_drag_->drag_begin();
    drop_begin(component_in_drag_.get());
}

void Window::remove_drag() {
    if (component_in_drag_) {
        component_in_drag_->drag_end();
        drop_end(component_in_drag_.get());
        component_in_drag_.reset();
    }
}

void Window::complete_drag(Component *component) {
    if (!component_in_drag_ || component == component_in_drag_->parent()) {
        return;
    }
    if (component->accept_drop(component_in_drag_.get()) && component_in_drag_->accept_drag(component) ) {
        component->complete_drop(component_in_drag_.get());
    }
}

void Window::handle_mouse_middle_released(int x, int y) {
    auto released_ptr = component_in_mouse_down_middle_.get();
    if (component_in_mouse_down_middle_) {
        component_in_mouse_down_middle_->handle_mouse_middle_released(x - component_in_mouse_down_middle_->abs_x(), y - component_in_mouse_down_middle_->abs_y());
        component_in_mouse_down_middle_.reset();
    }
    auto component = component_at_mouse(x, y);
    if (component) {
        if (component != released_ptr) {
            component->handle_mouse_middle_released(x, y);
        }
    }
}

void Window::handle_mouse_right_released(int x, int y) {
    auto released_ptr = component_in_mouse_down_right_.get();
    if (component_in_mouse_down_right_) {
        component_in_mouse_down_right_->handle_mouse_middle_released(x - component_in_mouse_down_right_->abs_x(), y - component_in_mouse_down_right_->abs_y());
        component_in_mouse_down_right_.reset();
    }
    auto component = component_at_mouse(x, y);
    if (component) {
        if (component != released_ptr) {
            component->handle_mouse_right_released(x, y);
        }
    }
}

void Window::handle_mouse_moved(int x, int y) {
    if (component_in_mouse_down_right_) {
        component_in_mouse_down_right_->handle_mouse_moved(x - component_in_mouse_down_right_->abs_x(), y - component_in_mouse_down_right_->abs_y());
    }
    if (component_in_mouse_down_left_) {
        component_in_mouse_down_left_->handle_mouse_moved(x - component_in_mouse_down_left_->abs_x(), y - component_in_mouse_down_left_->abs_y());
    }
    if (component_in_mouse_down_middle_) {
        component_in_mouse_down_middle_->handle_mouse_moved(x - component_in_mouse_down_middle_->abs_x(), y - component_in_mouse_down_middle_->abs_y());
    }
    mouse_move_x_ = x;
    mouse_move_y_ = y;
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
    if (!component_in_drag_) {
        return;
    }
    int coord_x = (mouse_move_x_ - mouse_down_x_);
    int coord_y = (mouse_move_y_ - mouse_down_y_);
    component_in_drag_->set_drag_coord(coord_x, coord_y);
}

void Window::handle_keypressed(int key) {
    if (component_in_focus_) {
        component_in_focus_->handle_keypressed(key);
    }
}

void Window::handle_mouse_wheel(int8_t direction, int x, int y) {
    auto component = component_at_mouse(x, y);
    if (component) {
        component->handle_mouse_wheel(direction, x, y);
    }
}

void Window::replace_mouse(Component *component) {
    if (component_in_mouse_.get() == component) {
        return;
    }
    if (component_in_mouse_) {
        component_in_mouse_->mouse_exit();
    }
    component_in_mouse_ = component->share();
    component_in_mouse_->mouse_enter();
}

void Window::remove_mouse() {
    if (component_in_mouse_) {
        component_in_mouse_->mouse_exit();
        component_in_mouse_.reset();
    }
}

void Window::update_cursor(void *render_window) {
    static auto cur_arrow = sf::Cursor::loadFromSystem(sf::Cursor::Type::Arrow);
    static auto cur_hand = sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand);
    static auto cur_sizeall = sf::Cursor::loadFromSystem(sf::Cursor::Type::SizeAll);
    static auto cur_edit = sf::Cursor::loadFromSystem(sf::Cursor::Type::Text);
    
    auto wnd = static_cast<sf::RenderWindow *>(render_window);
    sf::Cursor::Type cursor_type = sf::Cursor::Type::Arrow;
    if (cur_arrow && cur_hand && cur_sizeall && cur_edit) {
        if (component_in_drag_) {
            wnd->setMouseCursor(*cur_sizeall);
        } else if (component_in_mouse_) {
            if (component_in_mouse_->cursor() == cursor_drag) {
                wnd->setMouseCursor(*cur_sizeall);
            } else if (component_in_mouse_->cursor() == cursor_hand) {
                wnd->setMouseCursor(*cur_hand);
            } else if (component_in_mouse_->cursor() == cursor_edit) {
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
    for (auto & i : floating_components_) {
        if (x >= i->abs_x() && y >= i->abs_y() && x <= i->abs_x() + i->abs_w() && y <= i->abs_y() + i->abs_h()) {
            if (auto c = i->find_top_clickable(x, y)) {
                if (floatting) {
                    *floatting = i.get();
                }
                return c;
            }
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
    for (auto & i : floating_components_) {
        if (comp == i.get()) {
            return;
        }
    }
    floating_components_.push_front(comp->share());
}

void Window::remove_floating_commponent(Component *comp) {
    if (comp == this) return;
    for (auto it = floating_components_.begin(); it != floating_components_.end(); it++) {
        if (it->get() == comp) {
            it->get()->handle_float_off();
            floating_components_.erase(it);
            return;
        }
    }
}

void Window::pop_front_floatting_components(Component *floatting) {
    while(floating_components_.begin() != floating_components_.end()) {
        if (floating_components_.begin()->get() == floatting) {
            return;
        }
        floating_components_.begin()->get()->handle_float_off();
        floating_components_.erase(floating_components_.begin());
    }
}


std::shared_ptr<Window> window_new(int w, int h, const char *title) {
    return std::make_shared<Window>(Window(w, h, title));
}



} // namespace dfe
