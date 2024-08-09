#pragma once

#include <string>
#include "simple-ui/component.h"
#include "simple-ui/arrow_button.h"
#include "simple-ui/listbox.h"

namespace sf {
    class Text;
}

namespace dfe_ui
{

class Combobox : public Component {
  public:
    Combobox(Window * window, int x, int y, int w, int h);
    virtual ~Combobox();
    virtual void paint(sf::RenderTarget *render_target) override;
    std::wstring text();
    int character_size();
    void character_size(int value);
    void text_color(uint32_t color);
    uint32_t text_color();
    void fill_color(uint32_t color);
    uint32_t fill_color();
    void add(const std::wstring& value);
    void remove(size_t index);
    void clear();
    std::wstring item(size_t index);
    size_t size();

  private:
    void update_text_min_y();
    void show_list();

 protected:
    void handle_parent_resized() override;
    void handle_mouse_wheel(int8_t direction, int x, int y) override;
    void handle_mouse_left_pressed(int x, int y) override;
    void handle_click() override;
    bool clickable() override;
    bool focusable() override;

  private:
    vertical_text_alignment_t     m_text_valign = text_alignment_middle;
    std::shared_ptr<sf::Text>         m_text;
    std::shared_ptr<Listbox>      m_list;
    std::shared_ptr<ArrowButton>  m_button;
    bool                          m_floatting_list = false;
    int                           m_text_min_y = 0;
    int                           m_character_size = 30;
    uint32_t                      m_text_color = 0;
    uint32_t                      m_fill_color = 0;
    uint32_t                      m_outline_color = 0;
    uint32_t                      m_arrow_color = 0xFFFFFFFF;
    uint32_t                      m_highlighted_color = 0xFFFFFFFF;
    uint32_t                      m_pressed_color = 0x666666FF;
    uint32_t                      m_selected_color = 0x000000FF;
    uint32_t                      m_selected_text_color = 0xFFFFFFFF;

};

} // namespace dfe_ui
