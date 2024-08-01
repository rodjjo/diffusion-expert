#pragma once

#include <functional>

#include "simple-ui/panel.h"

namespace dfe_ui
{

typedef std::function<bool(Component *self, Component *component)> cb_accept_component_t;
typedef std::function<void(Component *self, Component *component)> cb_complete_drop_t;
typedef std::function<void(Component *self)> cb_dragdrop_hint_t;

class DragDrop : public Panel {
    public:
        DragDrop(Window * window, int x, int y, int w, int h);
        virtual ~DragDrop();
        void set_on_accept_drop(cb_accept_component_t cb);
        void set_on_accept_drag(cb_accept_component_t cb);
        void set_on_drag_begin(cb_dragdrop_hint_t cb);
        void set_on_drag_end(cb_dragdrop_hint_t cb);
        void set_on_drop_begin(cb_dragdrop_hint_t cb);
        void set_on_drop_end(cb_dragdrop_hint_t cb);
        void set_on_complete_drop(cb_complete_drop_t cb);
        void drag_enabled(bool value);
        void drop_enabled(bool value);
        bool drag_enabled() override;
        bool drop_enabled() override;
        component_status_t status() override;
        component_cursor_t cursor() override;
    protected:
        bool accept_drag(Component *comp) override;
        bool accept_drop(Component *comp) override;
        void complete_drop(Component *comp) override;
        bool clickable() override;

        void drag_begin() override;
        void drag_end() override;
        void drop_begin() override;
        void drop_end() override;

    private:
        cb_complete_drop_t m_cb_on_complete_drop;
        cb_accept_component_t m_cb_accept_drop;
        cb_accept_component_t m_cb_accept_drag;
        cb_dragdrop_hint_t m_cb_drag_begin;
        cb_dragdrop_hint_t m_cb_drag_end;
        cb_dragdrop_hint_t m_cb_drop_begin;
        cb_dragdrop_hint_t m_cb_drop_end;
        component_status_t m_status = component_status_normal;
        bool m_drag_enabled = false;
        bool m_drop_enabled = false;
};
    
} // namespace dfe_ui
