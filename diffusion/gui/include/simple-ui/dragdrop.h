#pragma once

#include <functional>

#include "simple-ui/panel.h"

namespace dfe_ui
{

typedef std::function<bool(Component *self, Component *component)> cb_accept_component_t;
typedef std::function<void(Component *self)> cb_dragdrop_hint_t;

class DragDrop : public Panel {
    public:
        DragDrop(int x, int y, int w, int h);
        virtual ~DragDrop();
        void set_on_accept_drop(cb_accept_component_t cb);
        void set_on_accept_drag(cb_accept_component_t cb);
        void set_on_drag_begin(cb_dragdrop_hint_t cb);
        void set_on_drag_end(cb_dragdrop_hint_t cb);
        void set_on_drop_begin(cb_dragdrop_hint_t cb);
        void set_on_drop_end(cb_dragdrop_hint_t cb);
        void drag_enabled(bool value);
        void drop_enabled(bool value);
        bool drag_enabled() override;
        bool drop_enabled() override;
        component_status_t status() override;

    protected:
        bool accept_drag(Component *comp);
        bool accept_drop(Component *comp);
        bool clickable() override;

        void drag_begin() override;
        void drag_end() override;
        void drop_begin() override;
        void drop_end() override;

    private:
        cb_accept_component_t cb_accept_drop_;
        cb_accept_component_t cb_accept_drag_;
        cb_dragdrop_hint_t cb_drag_begin_;
        cb_dragdrop_hint_t cb_drag_end_;
        cb_dragdrop_hint_t cb_drop_begin_;
        cb_dragdrop_hint_t cb_drop_end_;
        component_status_t status_ = component_status_normal;
        bool drag_enabled_ = false;
        bool drop_enabled_ = false;
};
    
} // namespace dfe_ui
