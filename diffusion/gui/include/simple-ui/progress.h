#pragma once

#include <string>
#include "component.h"

namespace sf {
    class Text;
}

namespace dfe_ui
{

class ProgressBar : public Component {
  public:
    ProgressBar(Window * window, int x, int y, int w, int h);
    virtual ~ProgressBar();
    virtual void paint(sf::RenderTarget *render_target) override;

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
    std::shared_ptr<sf::Text>         m_text;
    int                           m_text_min_y = 0;
    int                           m_character_size = 30;
    uint32_t                      m_fill_color = 0xFFFFFFFF;
    uint32_t                      m_outline_color = 0x000000FF;
    uint32_t                      m_progress_color = 0x000000FF;
    uint32_t                      m_text_color = 0;
    int64_t                       m_progress = 0;
    int64_t                       m_progress_max = 100;
};    

} // namespace dfe_gui
