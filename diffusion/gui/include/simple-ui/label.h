#pragma once

#include <string>
#include "component.h"

namespace sf {
    class Text;
}

namespace dfe_ui
{

class Label : public Component {
  public:
    Label(Window * window, int x, int y, int w, int h, const std::wstring &text);
    virtual ~Label();
    virtual void paint(sf::RenderTarget *render_target) override;
    std::wstring text();
    void text(const std::wstring &value);
    int character_size();
    void character_size(int value);
    void text_color(uint32_t color);
    uint32_t text_color();

  private:
    void update_text_min_y();

  private:
    vertical_text_alignment_t     m_text_valign = text_alignment_middle;
    std::shared_ptr<sf::Text>         m_text;
    int                           m_text_min_y = 0;
    int                           m_character_size = 30;
    uint32_t                      m_text_color = RGBA_TO_COLOR(255, 255, 255, 255);
};

} // namespace dfe_ui
