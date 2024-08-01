#pragma once

#include <functional>
#include <string>

#include "simple-ui/edit.h"


namespace dfe_ui
{

class TextBox : public Edit {
  public:
    TextBox(Window * window, int x, int y, int w, int h);
    virtual ~TextBox();
    virtual void paint(sf::RenderTarget *render_target) override;
    void color(uint32_t value);
    uint32_t color();
    void outline_color(uint32_t value);
    uint32_t outline_color();

 private:
    uint32_t m_color = 0xFFFFFFFF;
    uint32_t m_outline_color = 0x000000FF;
};
    
} // namespace dfe_ui
