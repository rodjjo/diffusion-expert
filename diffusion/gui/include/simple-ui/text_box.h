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
    virtual void paint(void *render_window) override;
    void color(uint32_t value);
    uint32_t color();
    void outline_color(uint32_t value);
    uint32_t outline_color();

 private:
    uint32_t color_ = 0xFFFFFFFF;
    uint32_t outline_color_ = 0x000000FF;
};
    
} // namespace dfe_ui
