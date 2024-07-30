#pragma once

#include <string>
#include "simple-ui/component.h"
#include "simple-ui/arrow_button.h"
#include "simple-ui/listbox.h"

namespace dfe_ui
{

class Combobox : public Component {
  public:
    Combobox(Window * window, int x, int y, int w, int h);
    virtual ~Combobox();
    virtual void paint(void *render_window) override;
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
    std::wstring at(size_t index);
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
    vertical_text_alignment_t     text_valign_ = text_alignment_middle;
    std::shared_ptr<void>         text_;
    std::shared_ptr<Listbox>      list_;
    std::shared_ptr<ArrowButton>  button_;
    bool                          floatting_list_ = false;
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
