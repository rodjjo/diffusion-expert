#pragma once

#include <functional>
#include "simple-ui/component.h"

namespace dfe_ui
{

class Panel;

typedef std::function<void(Panel *)> cb_event_t;

class Panel : public Component {
  public:
    Panel(Window * window, int x, int y, int w, int h);
    virtual ~Panel();
    virtual void paint(sf::RenderTarget *render_target) override;
    void bg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void fg_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint8_t bg_color_r();
    uint8_t bg_color_g();
    uint8_t bg_color_b();
    uint8_t bg_color_a();

    uint8_t fg_color_r();
    uint8_t fg_color_g();
    uint8_t fg_color_b();
    uint8_t fg_color_a();

    void set_on_mouse_enter(cb_event_t cb);
    void set_on_mouse_exit(cb_event_t cb);

  protected:
    void mouse_enter() override;
    void mouse_exit() override;
    
  private:
    uint32_t m_fg_color = 0;
    uint32_t m_bg_color = 0;
    cb_event_t m_on_mouse_enter;
    cb_event_t m_on_mouse_exit;
};
    
} // namespace dfe
