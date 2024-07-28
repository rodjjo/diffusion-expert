#pragma once

#include "simple-ui/component.h"

namespace dfe_ui
{

class Combobox : public Component {
  public:
    Combobox(Window * window, int x, int y, int w, int h, const std::wstring &text);
    virtual ~Combobox();
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
    uint32_t                      text_color_ = 0;
    uint32_t                      fill_color_ = 0;
    uint32_t                      outline_color_ = 0;
    uint32_t                      arrow_color_ = 0xFFFFFFFF;
    uint32_t                      highlighted_color_ = 0xFFFFFFFF;
    uint32_t                      pressed_color_ = 0x666666FF;
    uint32_t                      selected_color_ = 0x000000FF;
    uint32_t                      selected_text_color_ = 0xFFFFFFFF;

};

} // namespace dfe_ui
