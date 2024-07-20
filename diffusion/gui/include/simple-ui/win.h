#pragma once

#include "component.h"

namespace dfe_ui
{
    
class Window : public Component {
  public:
    Window(unsigned int w, unsigned int h, const char *title);
    virtual ~Window();
    void run();

  protected:
    void handle_parent_resized() override;
    void handle_textentered(uint32_t unicode) override;
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
    void replace_focus(Component *component);
    void replace_drag(Component *component);

  private:
    std::shared_ptr<void> window_;
    bool mouse_left_pressed_ = false;
    bool mouse_middle_pressed_ = false;
    bool mouse_right_pressed_ = false;
    std::shared_ptr<Component> component_in_focus_;
    std::shared_ptr<Component> component_in_drag_;
    std::shared_ptr<Component> component_in_drop_;

};


std::shared_ptr<Window> window_new(int w, int , const char *title);

} // namespace dfe
