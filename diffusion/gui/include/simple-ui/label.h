#pragma once

#include <string>
#include "component.h"

namespace dfe_ui
{

class Label : public Component {
  public:
    Label(Window * window, int x, int y, int w, int h, const std::wstring &text);
    virtual ~Label();
    virtual void paint(void *render_window) override;
    std::wstring text();
    void text(const std::wstring &value);
    int character_size();
    void character_size(int value);
    void text_color(uint32_t color);
    uint32_t text_color();

  private:
    void update_text_min_y();

  private:
    vertical_text_alignment_t     text_valign_ = text_alignment_middle;
    std::shared_ptr<void>         text_;
    int                           text_min_y_ = 0;
    int                           character_size_ = 30;
    uint32_t                      text_color_ = RGBA_TO_COLOR(255, 255, 255, 255);
};

} // namespace dfe_ui
