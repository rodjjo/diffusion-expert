#include "src/data/xpm.h"
#include "src/dialogs/common_dialogs.h"
#include "src/dialogs/size_dialog.h"


namespace dexpert
{

SizeWindow::SizeWindow(const char *title, bool single_value) : Fl_Window(0, 0, 300, 100, title) {
    single_value_ = single_value;
    this->position(Fl::w()/ 2 - this->w() / 2,  Fl::h() / 2 - this->h() / 2);
    this->set_modal();

    width_ = new Fl_Int_Input(5, 25, 120, 30, single_value ? "Dimension:" : "Width:");
    height_  = new Fl_Int_Input(width_->x() + width_->w() + 5, 25, 120, 30, "Height:");
    btn_ok_.reset(new Button(xpm::image(xpm::button_ok_16x16), [this] {
        confirmOk();
    }));
    btn_cancel_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
        this->hide();
    }));
    width_->align(FL_ALIGN_TOP_LEFT);
    height_->align(FL_ALIGN_TOP_LEFT);
    btn_ok_->tooltip("Confirm and close.");
    btn_cancel_->tooltip("Cancel and close.");
    btn_cancel_->size(60, 30);
    btn_ok_->size(60, 30);
    btn_cancel_->position(w() - 5 - btn_cancel_->w(), h() - 5 - btn_cancel_->h());
    btn_ok_->position(btn_cancel_->x() - 5 - btn_ok_->w(), btn_cancel_->y());
    if (single_value) {
        height_->hide();
    }
}

SizeWindow::~SizeWindow() {
}

void SizeWindow::setInitialSize(int x, int y) {
    char buffer[100] = {0,};
    sprintf(buffer, "%d", x);
    width_->value(buffer);
    sprintf(buffer, "%d", y);
    height_->value(buffer);
}

void SizeWindow::retriveSize(int *x, int *y) {
    if (x != NULL) {
        sscanf(width_->value(), "%d", x);
    }
    if (y != NULL) {
        sscanf(height_->value(), "%d", y);
    }
}

void SizeWindow::confirmOk() {
    int x = 0;
    int y = 0;
    retriveSize(&x, &y);
    if (x <= 0 || x > 7200) {
        show_error("Invalid width. The value should from 1 to 7200");
        return;
    }
    if (y <= 0 || y > 7200) {
        show_error("Invalid height. The value should from 1 to 7200");
        return;
    }
    ok_confirmed_ = true;
    hide();
}

bool SizeWindow::run() {
    this->show();
    while (this->shown()) {
        Fl::wait(0.01);
    }
    return ok_confirmed_;
}

bool getSizeFromDialog(const char *title, int *x, int *y) {
    SizeWindow *wnd = new SizeWindow(title, y == NULL);
    wnd->setInitialSize(*x, y != NULL ? *y : 512);
    bool result = wnd->run();
    if (result) {
        wnd->retriveSize(x, y);
    }
    Fl::delete_widget(wnd);
    Fl::do_widget_deletion();
    return result;
}

bool getSizeFromDialog(const char *title, int *x) {
    return getSizeFromDialog(title, x, NULL);
}
    
} // namespace dexpert
