#pragma once

#include "simple-ui/component.h"

namespace dfe_ui
{

class Panel : public Component {
  public:
    Panel(int x, int y, int w, int h);
    virtual ~Panel();
    virtual void paint(void *render_window) override;
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

  private:
    uint32_t fg_color_ = 0;
    uint32_t bg_color_ = 0;
};
    
} // namespace dfe
