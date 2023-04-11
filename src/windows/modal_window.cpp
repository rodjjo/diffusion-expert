#include "src/windows/modal_window.h"

namespace dexpert
{

ModalWindow::ModalWindow(int xx, int yy, int ww, int hh, const char* title ): Fl_Window(xx, yy, ww, hh, title) {
    this->set_modal();
}

ModalWindow::~ModalWindow() {

}

void ModalWindow::enableExitShortcuts() {
    exit_shortcuts_ = true;
}

int ModalWindow::handle(int event) {
    if (!exit_shortcuts_) {
        switch (event) {
            case FL_KEYDOWN:
            case FL_KEYUP: {
                if (Fl::event_key() == FL_Escape) {
                    return  1;
                }
                if (Fl::event_key() == (FL_F + 4) && (Fl::event_state() & FL_ALT) != 0) {
                    return  1; // Do not allow ALT + F4
                }
            }
            break;
        }
    }
    return Fl_Window::handle(event);
}


} // namespace dexpert
