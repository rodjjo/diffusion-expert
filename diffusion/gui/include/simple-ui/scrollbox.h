#pragma once

#include "simple-ui/events.h"
#include "simple-ui/component.h"
#include "simple-ui/scrollbar.h"

namespace dfe_ui {

class Scrollbox : public Component {
 public:
    Scrollbox(Window * window, int x, int y, int w, int h);
    virtual ~Scrollbox();
    uint32_t fill_color();
    void fill_color(uint32_t value);
    void outline_color(uint32_t value);
    uint32_t outline_color();
    bool autoscroll();
    void autoscroll(bool value);
    void scroll_x(int value);
    void scroll_y(int value);
    void max_scroll_x(uint32_t value);
    void max_scroll_y(uint32_t value);
    uint32_t max_scroll_x();
    uint32_t max_scroll_y();
   virtual void paint(sf::RenderTarget *render_target) override;
   void add(std::shared_ptr<Component> child) override;
   Component & scroll_component();

 private:
    void compute_autoscroll();
    void sync_scrollbar();
    void update_scrollbar_range();
    void scroll_visibility();
    void positionate_scrollbars();

 protected:
    void handle_parent_resized() override;
    void handle_child_count_changed() override;
    
 private:
    bool m_inside_scroll_callback    = false;
    std::shared_ptr<Component>     m_scroll_component;
    std::shared_ptr<Scrollbar>     m_vertical_sb;
    std::shared_ptr<Scrollbar>     m_horizontal_sb;
    bool        m_autoscroll        = true;
    uint32_t    m_fill_color        = 0;
    uint32_t    m_outline_color     = 0;
    int         m_max_scroll_x      = 0;
    int         m_max_scroll_y      = 0;
};

}  // namespace dfe_ui
