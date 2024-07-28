#pragma once

#include <vector>
#include <memory>

#define RGBA_TO_COLOR(r, g, b, a) ((r) << 24) | ((g) << 16) | ((b) << 8)  | (a)
#define RGBA_R(color) ((color) >> 24) & 255
#define RGBA_G(color) ((color) >> 16) & 255
#define RGBA_B(color) ((color) >> 8) & 255
#define RGBA_A(color) (color) & 255

namespace dfe_ui {


typedef enum {
  component_status_normal = 0,
  component_status_dragging,
  component_status_dropping
} component_status_t;

typedef enum {
  text_alignment_top = 0,
  text_alignment_middle,
  text_alligment_bottom
} vertical_text_alignment_t;

typedef enum {
  cursor_arrow,
  cursor_drag,
  cursor_hand,
  cursor_edit
} component_cursor_t;

class Component;
class Window;

class ScissorContext {
  public:
    ScissorContext(int target_h, Component *component);
    ~ScissorContext();
    bool visible();
  private:
    bool disable_scissor_ = false;
    float view_x_ = 0;
    float view_y_ = 0;
    float view_w_ = 0;
    float view_h_ = 0;
};

class ComponentList {
  public:
    ComponentList(Component *parent);
    virtual ~ComponentList();
    Component & operator[](size_t index);
    size_t size();
    Component & at(size_t index);
    bool empty();
  private:
    Component *parent_;
    std::vector<std::shared_ptr<Component> > items_;
    void sort();

  private:
    void add(std::shared_ptr<Component> component);
    void remove(Component *element);
    friend class Component;
};

class Component : public std::enable_shared_from_this<Component>  {
  public:
    Component(Window *window);
    virtual ~Component();
    std::shared_ptr<Component> share();
    virtual void add(std::shared_ptr<Component> child);
    int zorder() const;
    void zorder(int value);
    virtual void paint(void *render_window);
    bool damaged();
    void damaged(bool value);
    static bool global_damaged();
    bool visible();
    void visible(bool value);
    bool enabled();
    void enabled(bool value);
    bool abs_enabled();

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

    int x();
    int y();
    int w();
    int h();
    int scroll_x();
    int scroll_y();
    float scale();
    float abs_scale();
    void x(int value);
    void y(int value);
    void w(int value);
    void h(int value);
    int abs_x();
    int abs_y();
    int abs_w();
    int abs_h();
    void scroll_x(int value);
    void scroll_y(int value);
    void scale(float value);
    void size(int w, int h);
    void coordinates(int x, int y, int w, int h);
    virtual bool clickable();
    virtual bool editable();
    virtual bool focusable();
    Component *parent();
    virtual Component *find_top_clickable(int &x, int &y);
    void paint_children(void *render_window, bool check_status=true);
    void set_drag_coord(int x, int y);

    ComponentList & items();
    size_t tag();
    void tag(size_t value);

    virtual component_status_t status();
    virtual component_cursor_t cursor();
    
    void float_on();
    void float_off();

    static int compute_text_min_y(void *text_shape);

   public:
    virtual void handle_parent_resized() {};
    virtual void handle_textentered(wchar_t unicode) {};
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

  protected:
    void fire_parent_resized();
  
  private:
    Window                    *window_ = NULL;
    ComponentList             items_;
    bool                      enabled_ = true;
    bool                      visible_ = true;
    bool                      damaged_ = true;
    int                       z_order_ = 0;
    int                       scroll_x_ = 0;
    int                       scroll_y_ = 0;
    float                     scale_ = 1.0;
    int                       x_ = 0;
    int                       y_ = 0;
    int                       w_ = 0;
    int                       h_ = 0;
    int                       drag_x_ = 0;
    int                       drag_y_ = 0;
    size_t                    tag_ = 0;

  private:
    friend class ComponentList;
    Component                 *parent_ = NULL;
};


}