#pragma once

#include <string>
#include "component.h"


namespace dfe_ui
{

class ProgressBar : public Component {
  public:
    ProgressBar(Window * window, int x, int y, int w, int h);
    virtual ~ProgressBar();
    virtual void paint(void *render_window) override;

    int64_t progress();
    void progress(int64_t value);
    int progress_max(int64_t value);
    int64_t progress_max();

    int character_size();
    void character_size(int value);
    void text_color(uint32_t color);
    uint32_t text_color();
    void fill_color(uint32_t color);
    uint32_t fill_color();
    void outline_color(uint32_t color);
    uint32_t outline_color();
    void progress_color(uint32_t color);
    uint32_t progress_color();

  private:
    void update_text_min_y();

  private:
    std::shared_ptr<void>         text_;
    int                           text_min_y_ = 0;
    int                           character_size_ = 30;
    uint32_t                      fill_color_ = 0xFFFFFFFF;
    uint32_t                      outline_color_ = 0x000000FF;
    uint32_t                      progress_color_ = 0x000000FF;
    uint32_t                      text_color_ = 0;
    int64_t                       progress_ = 0;
    int64_t                       progress_max_ = 100;
};    

} // namespace dfe_gui
