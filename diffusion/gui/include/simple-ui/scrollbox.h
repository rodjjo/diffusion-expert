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
   virtual void paint(void *render_window) override;
   void add(std::shared_ptr<Component> child) override;
   ComponentList & scroll_items();

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
    bool inside_scroll_callback_    = false;
    std::shared_ptr<Component>     scroll_component_;
    std::shared_ptr<Scrollbar>     vertical_sb_;
    std::shared_ptr<Scrollbar>     horizontal_sb_;
    bool        autoscroll_        = true;
    uint32_t    fill_color_        = 0;
    uint32_t    outline_color_     = 0;
    int         max_scroll_x_      = 0;
    int         max_scroll_y_      = 0;
};

}  // namespace dfe_ui
