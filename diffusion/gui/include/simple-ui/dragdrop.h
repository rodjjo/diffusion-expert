#pragma once

#include <functional>

#include "simple-ui/panel.h"

namespace dfe_ui
{

typedef std::function<bool(Component *self, Component *component)> cb_accept_component_t;

class DragDrop : public Panel {
    public:
        DragDrop(int x, int y, int w, int h);
        virtual ~DragDrop();
        void set_on_accept_drop(cb_accept_component_t cb);
        void set_on_accept_drag(cb_accept_component_t cb);
        void drag_enabled(bool value);
        void drop_enabled(bool value);
        bool drag_enabled() override;
        bool drop_enabled() override;

    protected:
        bool accept_drag(Component *comp);
        bool accept_drop(Component *comp);

    private:
        cb_accept_component_t cb_accept_drop_;
        cb_accept_component_t cb_accept_drag_;
        bool drag_enabled_ = false;
        bool drop_enabled_ = false;
};
    
} // namespace dfe_ui
