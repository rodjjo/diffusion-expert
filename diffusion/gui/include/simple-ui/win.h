#pragma once

#include <memory>
#include <list>
#include "component.h"


namespace dfe_ui
{
    
class Window : public Component {
  public:
    Window(unsigned int w, unsigned int h, const char *title);
    virtual ~Window();
    void run();

  private:
    friend class Component;
    void add_floating_commponent(Component *comp);
    void remove_floating_commponent(Component *comp);
    bool is_floatting_component(Component *comp);

  protected:
    void handle_parent_resized() override;
    void handle_text_entered(wchar_t unicode) override;
    void handle_mouse_left_pressed(int x, int y) override;
    void handle_mouse_middle_pressed(int x, int y) override;
    void handle_mouse_right_pressed(int x, int y) override;
    void handle_mouse_left_released(int x, int y) override;
    void handle_mouse_middle_released(int x, int y) override;
    void handle_mouse_right_released(int x, int y) override;
    void handle_mouse_moved(int x, int y) override;
    void handle_keypressed(int key) override;
    void handle_mouse_wheel(int8_t direction, int x, int y) override;

  private:
    void remove_focus();
    void remove_drag();
    void remove_mouse();
    void replace_focus(Component *component);
    void replace_drag(Component *component);
    void replace_mouse(Component *component);
    void complete_drag(Component *component);
    void update_drag_coord();
    void update_cursor(sf::RenderTarget *render_target);
    Component *component_at_mouse(int &x, int &y, Component **floatting);
    Component *component_at_mouse(int &x, int &y);
    void fix_mouse_coords(int &x, int &y);
    void pop_front_floatting_components(Component *floatting);

  private:
    std::list<std::shared_ptr<Component> > m_floating_components;
    std::shared_ptr<sf::RenderWindow> m_window;
    bool m_mouse_left_pressed = false;
    bool m_mouse_middle_pressed = false;
    bool m_mouse_right_pressed = false;
    int m_mouse_down_x = 0;
    int m_mouse_down_y = 0;
    int m_mouse_move_x = 0;
    int m_mouse_move_y = 0;
    std::shared_ptr<Component> m_component_in_focus;
    std::shared_ptr<Component> m_component_in_drag;
    std::shared_ptr<Component> m_component_in_drop;
    std::shared_ptr<Component> m_component_in_mouse;
    std::shared_ptr<Component> m_component_in_mouse_down_left;
    std::shared_ptr<Component> m_component_in_mouse_down_middle;
    std::shared_ptr<Component> m_component_in_mouse_down_right;
};


std::shared_ptr<Window> window_new(int w, int , const char *title);

} // namespace dfe
