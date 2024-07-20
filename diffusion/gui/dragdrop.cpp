#include "simple-ui/dragdrop.h"

namespace dfe_ui
{
    DragDrop::DragDrop(int x, int y, int w, int h) : Panel(x, y, w, h) {
    }

    DragDrop::~DragDrop() {

    }

    void DragDrop::set_on_accept_drop(cb_accept_component_t cb) {
        cb_accept_drag_ = cb;
    }

    void DragDrop::set_on_accept_drag(cb_accept_component_t cb) {
        cb_accept_drop_ = cb;
    }

    void DragDrop::drag_enabled(bool value) {
        drag_enabled_ = value;
    }

    void DragDrop::drop_enabled(bool value) {
        drop_enabled_ = value;
    }

    bool DragDrop::drag_enabled() {
        return drag_enabled_;
    }

    bool DragDrop::drop_enabled() {
        return drop_enabled_;
    }

    bool DragDrop::accept_drag(Component *comp) {
        if (cb_accept_drag_) {
            return cb_accept_drag_(this, comp);
        }
        return false;
    }

    bool DragDrop::accept_drop(Component *comp) {
        if (cb_accept_drop_) {
            return cb_accept_drop_(this, comp);
        }
        return false;
    }


} // namespace dfe_ui
