#include <stdio.h>

#include "simple-ui/dragdrop.h"

namespace dfe_ui
{
    DragDrop::DragDrop(Window * window, int x, int y, int w, int h) : Panel(window, x, y, w, h) {
    }

    DragDrop::~DragDrop() {

    }

    void DragDrop::set_on_accept_drop(cb_accept_component_t cb) {
        m_cb_accept_drop = cb;
    }

    void DragDrop::set_on_accept_drag(cb_accept_component_t cb) {
        m_cb_accept_drag = cb;
    }

    void DragDrop::drag_enabled(bool value) {
        m_drag_enabled = value;
    }

    void DragDrop::drop_enabled(bool value) {
        m_drop_enabled = value;
    }

    bool DragDrop::drag_enabled() {
        return m_drag_enabled;
    }

    bool DragDrop::drop_enabled() {
        return m_drop_enabled;
    }

    bool DragDrop::accept_drag(Component *comp) {
        if (m_cb_accept_drag) {
            return m_cb_accept_drag(this, comp);
        }
        return false;
    }

    bool DragDrop::accept_drop(Component *comp) {
        if (m_cb_accept_drop) {
            return m_cb_accept_drop(this, comp);
        }
        return false;
    }

    void DragDrop::set_on_drag_begin(cb_dragdrop_hint_t cb) {
        m_cb_drag_begin = cb;
    }

    void DragDrop::set_on_drag_end(cb_dragdrop_hint_t cb) {
        m_cb_drag_end = cb;
    }

    void DragDrop::set_on_drop_begin(cb_dragdrop_hint_t cb) {
        m_cb_drop_begin = cb;
    }

    void DragDrop::set_on_drop_end(cb_dragdrop_hint_t cb) {
        m_cb_drop_end = cb;
    }

    void DragDrop::drag_begin() {
        m_status = component_status_dragging;
        if (m_cb_drag_begin) {
            m_cb_drag_begin(this);
        }
    }

    void DragDrop::drag_end() {
        m_status = component_status_normal;
        if (m_cb_drag_end) {
            m_cb_drag_end(this);
        }
    }

    void DragDrop::drop_begin() {
        m_status = component_status_dropping;
        if (m_cb_drop_begin) {
            m_cb_drop_begin(this);
        }
    }

    void DragDrop::drop_end() {
        m_status = component_status_normal;
        if (m_cb_drop_end) {
            m_cb_drop_end(this);
        }
    }
    
    bool DragDrop::clickable() {
        return true;
    };

    component_status_t DragDrop::status() {
        return m_status;
    }

    void DragDrop::set_on_complete_drop(cb_complete_drop_t cb) {
        m_cb_on_complete_drop = cb;
    }

    void DragDrop::complete_drop(Component *comp) {
        if (m_cb_on_complete_drop) {
            m_cb_on_complete_drop(this, comp);
        }
    }

    component_cursor_t DragDrop::cursor() {
        return m_drag_enabled ? cursor_drag : cursor_arrow;
    }


} // namespace dfe_ui
