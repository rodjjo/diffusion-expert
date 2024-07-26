#include <stdio.h>

#include "simple-ui/dragdrop.h"

namespace dfe_ui
{
    DragDrop::DragDrop(Window * window, int x, int y, int w, int h) : Panel(window, x, y, w, h) {
    }

    DragDrop::~DragDrop() {

    }

    void DragDrop::set_on_accept_drop(cb_accept_component_t cb) {
        cb_accept_drop_ = cb;
    }

    void DragDrop::set_on_accept_drag(cb_accept_component_t cb) {
        cb_accept_drag_ = cb;
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

    void DragDrop::set_on_drag_begin(cb_dragdrop_hint_t cb) {
        cb_drag_begin_ = cb;
    }

    void DragDrop::set_on_drag_end(cb_dragdrop_hint_t cb) {
        cb_drag_end_ = cb;
    }

    void DragDrop::set_on_drop_begin(cb_dragdrop_hint_t cb) {
        cb_drop_begin_ = cb;
    }

    void DragDrop::set_on_drop_end(cb_dragdrop_hint_t cb) {
        cb_drop_end_ = cb;
    }

    void DragDrop::drag_begin() {
        status_ = component_status_dragging;
        if (cb_drag_begin_) {
            cb_drag_begin_(this);
        }
    }

    void DragDrop::drag_end() {
        status_ = component_status_normal;
        if (cb_drag_end_) {
            cb_drag_end_(this);
        }
    }

    void DragDrop::drop_begin() {
        status_ = component_status_dropping;
        if (cb_drop_begin_) {
            cb_drop_begin_(this);
        }
    }

    void DragDrop::drop_end() {
        status_ = component_status_normal;
        if (cb_drop_end_) {
            cb_drop_end_(this);
        }
    }
    
    bool DragDrop::clickable() {
        return true;
    };

    component_status_t DragDrop::status() {
        return status_;
    }

    void DragDrop::set_on_complete_drop(cb_complete_drop_t cb) {
        cb_on_complete_drop_ = cb;
    }

    void DragDrop::complete_drop(Component *comp) {
        if (cb_on_complete_drop_) {
            cb_on_complete_drop_(this, comp);
        }
    }

    component_cursor_t DragDrop::cursor() {
        return drag_enabled_ ? cursor_drag : cursor_arrow;
    }


} // namespace dfe_ui
