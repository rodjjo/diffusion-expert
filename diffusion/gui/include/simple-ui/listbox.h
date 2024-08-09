#pragma once

#include <vector>
#include <string>

#include "simple-ui/events.h"
#include "simple-ui/component.h"
#include "simple-ui/scrollbar.h"

namespace sf {
    class Text;
}

namespace dfe_ui
{

class Listbox : public Component {
 public:
    Listbox(Window * window, int x, int y, int w, int h);
    virtual ~Listbox();
    void add(const std::wstring& value);
    void remove(size_t index);
    void clear();
    std::wstring item(size_t index);
    size_t size();
    int character_size();
    void character_size(int value);
    void text_color(uint32_t color);
    uint32_t text_color();
    void paint(sf::RenderTarget *render_target);
    void selected_text_color(int32_t color);
    uint32_t selected_text_color();
    void selected_color(int32_t color);
    uint32_t selected_color();
    void selected(size_t value);
    size_t selected();
    void scrollbar_width(int value);
    int scrollbar_width();
    void onchange(component_event_t value);
    component_event_t onchange();
    void onclick(component_event_t value);
    component_event_t onclick();

 protected:
    void handle_parent_resized() override;
    void handle_mouse_wheel(int8_t direction, int x, int y) override;
    void handle_mouse_left_pressed(int x, int y) override;
    bool clickable() override;
    bool focusable() override;

 private:
   void scrollbar_changed();
   void update_scrollbar();
   void update_text_min_y();
   int item_height();
   int visible_items();
   void fill_color(uint32_t value);
   uint32_t fill_color();
   void outline_color(uint32_t value);
   uint32_t outline_color();

 private:
   component_event_t             m_onchange;
   component_event_t             m_onclick;
   bool                          m_changing_scrollbar = false;
   uint32_t                      m_fill_color = 0xFFFFFFFF;
   uint32_t                      m_outline_color = 0x000000FF;
   std::shared_ptr<sf::Text>         m_text;
   size_t                        m_selected_index = 0;
   size_t                        m_top_element = 0;
   int                           m_text_min_y = 0;
   int                           m_character_size = 30;
   uint32_t                      m_text_color = 0;
   uint32_t                      m_selected_color = 0x000000FF;
   uint32_t                      m_selected_text_color = 0xffffffff;
   std::vector<std::wstring>     m_items;
   std::shared_ptr<Scrollbar>    m_scrollbar;
};


} // namespace dfe_ui
