#pragma once

#include <vector>
#include <memory>

namespace dfe_ui {

class Component;

class PaintingContext {
  public:
    PaintingContext(int target_h, Component *component);
    ~PaintingContext();
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
    Component();
    virtual ~Component();
    std::shared_ptr<Component> share();
    void add(std::shared_ptr<Component> child);
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
    bool drag_enabled();
    void drag_enabled(bool value);
    bool drop_enabled();
    void drop_enabled(bool value);
    int x();
    int y();
    int w();
    int h();
    void x(int value);
    void y(int value);
    void w(int value);
    void h(int value);
    void size(int w, int h);
    void coordinates(int x, int y, int w, int h);
    virtual bool clickable();
    virtual bool editable();
    virtual bool focusable();
    Component *find_top_clickable(int &x, int &y);
    void paint_children(void *render_window);
    int abs_x();
    int abs_y();
   public:
    virtual void handle_parent_resized();
    virtual void handle_textentered(uint32_t unicode);
    virtual void handle_mouse_left_pressed(int x, int y);
    virtual void handle_mouse_middle_pressed(int x, int y);
    virtual void handle_mouse_right_pressed(int x, int y);
    virtual void handle_mouse_left_released(int x, int y);
    virtual void handle_mouse_middle_released(int x, int y);
    virtual void handle_mouse_right_released(int x, int y);
    virtual void handle_mouse_moved(int x, int y);
    virtual void handle_keypressed(int key);
    virtual void handle_mouse_wheel(int8_t direction, int x, int y);
    virtual void handle_focus_lost();
    virtual void handle_focus_got();

  protected:
    void fire_parent_resized();

  private:
    ComponentList             items_;
    bool                      enabled_ = true;
    bool                      visible_ = true;
    bool                      damaged_ = true;
    bool                      drag_enabled_ = false;
    bool                      drop_enabled_ = false;
    int                       z_order_ = 0;
    int                       x_ = 0;
    int                       y_ = 0;
    int                       w_ = 0;
    int                       h_ = 0;

  private:
    friend class ComponentList;
    Component                 *parent_ = NULL;
};


}