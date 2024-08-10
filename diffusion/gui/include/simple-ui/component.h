#pragma once

#include <functional>
#include <vector>
#include <memory>

#define RGBA_TO_COLOR(r, g, b, a) ((r) << 24) | ((g) << 16) | ((b) << 8)  | (a)
#define RGBA_R(color) (((color) >> 24) & 255)
#define RGBA_G(color) (((color) >> 16) & 255)
#define RGBA_B(color) (((color) >> 8) & 255)
#define RGBA_A(color) ((color) & 255)

namespace sf {
    // sfml forward class declarations
    // just to avoid sfml's header inclusion
    class Font;
    class Text;
    class RenderTarget;
    class RenderWindow;
}

namespace dfe_ui {


typedef enum {
  component_status_normal,
  component_status_dragging,
  component_status_dropping
} component_status_t;

typedef enum {
  text_alignment_top,
  text_alignment_middle,
  text_alligment_bottom
} vertical_text_alignment_t;

typedef enum {
  cursor_arrow,
  cursor_drag,
  cursor_hand,
  cursor_edit
} component_cursor_t;

class Window;

class Component : public std::enable_shared_from_this<Component>  {
  public:
    Component(Window *window);
    virtual ~Component();
    std::shared_ptr<Component> share();
    int zorder() const;
    void zorder(int value);
    bool visible();
    void visible(bool value);
    bool enabled();
    void enabled(bool value);
    bool abs_enabled();

    Component & operator[](size_t index);
    size_t component_count();
    virtual Component & at(size_t index);

    virtual bool drag_enabled();
    virtual bool drop_enabled();

    virtual bool accept_drag(Component *comp);
    virtual bool accept_drop(Component *comp);
    virtual void complete_drop(Component *comp) {};
    
    virtual void drag_begin() {};
    virtual void drag_end() {};
    virtual void drop_begin() {};
    virtual void drop_end() {};

    virtual void mouse_enter() {};
    virtual void mouse_exit() {};
    
    void drop_begin(Component *source);
    void drop_end(Component *source);

    virtual void parent_changed() {};

    float scale();
    void scale(float value);
    int x();
    void x(int value);
    int y();
    void y(int value);
    int w();
    void w(int value);
    int h();
    void h(int value);
    int scroll_x();
    int scroll_y();
    virtual void scroll_x(int value);
    virtual void scroll_y(int value);
    float abs_scale();
    int abs_x();
    int abs_y();
    int abs_w();
    int abs_h();
    void size(int w, int h);
    void coordinates(int x, int y, int w, int h);
    virtual bool clickable();
    virtual bool editable();
    virtual bool focusable();
    Component *parent();
    virtual Component *find_top_clickable(int &x, int &y);
    void paint_children(sf::RenderTarget *render_target, bool check_status=true);
    void set_drag_coord(int x, int y);

    virtual void add(std::shared_ptr<Component> child);

    size_t tag();
    void tag(size_t value);

    virtual component_status_t status();
    virtual component_cursor_t cursor();
    
    virtual void float_on(Component *parent = NULL);
    virtual void float_off();

    static int compute_text_min_y(void *text_shape);
    bool is_floatting();

    void paint_constraint(int x, int y, int w, int h, int render_y, std::function<void()> cb);

    Window *window();

   protected:
    friend class Window;
    virtual void paint(sf::RenderTarget *render_target);
    
    virtual void handle_parent_resized() {};
    virtual void handle_child_count_changed() {};
    virtual void handle_text_entered(wchar_t unicode) {};
    virtual void handle_mouse_left_pressed(int x, int y) {};
    virtual void handle_mouse_middle_pressed(int x, int y) {};
    virtual void handle_mouse_right_pressed(int x, int y) {};
    virtual void handle_mouse_left_released(int x, int y) {};
    virtual void handle_mouse_middle_released(int x, int y) {};
    virtual void handle_mouse_right_released(int x, int y) {};
    virtual void handle_mouse_moved(int x, int y) {};
    virtual void handle_keypressed(int key) {};
    virtual void handle_mouse_wheel(int8_t direction, int x, int y) {};
    virtual void handle_focus_lost() {};
    virtual void handle_focus_got() {};
    virtual void handle_float_off() {};
    virtual void handle_click() {};

  protected:
    void fire_parent_resized();
    void fire_child_count_changed();

  private:
    int abs_scrollx();
    int abs_scrolly();

    void add_component(std::shared_ptr<Component> component);
    void remove_component(Component *element);
    void sort_components();

  private:
    std::vector<std::shared_ptr<Component> > m_items;
    Window                    *m_window = NULL;
    bool                      m_enabled = true;
    bool                      m_visible = true;
    int                       m_z_order = 0;
    int                       m_scroll_x = 0;
    int                       m_scroll_y = 0;
    float                     m_scale = 1.0;
    int                       m_x = 0;
    int                       m_y = 0;
    int                       m_w = 0;
    int                       m_h = 0;
    int                       m_drag_x = 0;
    int                       m_drag_y = 0;
    size_t                    m_tag = 0;

  private:
    friend class ComponentList;
    Component                 *m_floatting_parent = NULL;
    Component                 *m_parent = NULL;
};


}