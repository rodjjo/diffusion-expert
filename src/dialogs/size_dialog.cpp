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
    height_  = new Fl_Int_Input(width_->x() + width_->w() + 35, 25, 120, 30, "Height:");
    btn_proportion_.reset(new Button("<->", [this] {
        proportionalChanged();
    }));
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
    btn_proportion_->tooltip("Keep it proportional.");
    btn_proportion_->size(30, 30);
    btn_cancel_->size(60, 30);
    btn_ok_->size(60, 30);
    btn_proportion_->position(width_->x() + width_->w() + 2, width_->y());
    btn_cancel_->position(w() - 5 - btn_cancel_->w(), h() - 5 - btn_cancel_->h());
    btn_ok_->position(btn_cancel_->x() - 5 - btn_ok_->w(), btn_cancel_->y());
    if (single_value) {
        height_->hide();
        btn_proportion_->hide();
    }
    btn_proportion_->enableDownUp();
    btn_proportion_->down(true);
    width_->callback(&valueChangedCb, this);
    height_->callback(&valueChangedCb, this);
}

SizeWindow::~SizeWindow() {
}


void SizeWindow::proportionalChanged() {
    if (!btn_proportion_->down()) {
        return;
    }
    int x = 0, y = 0;
    sscanf(width_->value(), "%d", &x);
    sscanf(height_->value(), "%d", &y);
    if (x != 0 && y != 0) {
        if (x >= y) {
            proportion_ = y / (float)x;
            proportion_to_x_ = true;
        } else {
            proportion_ = x / (float)y;
            proportion_to_x_ = false;
        }
    } else {
        proportion_ = 1.0;
    }
}

void SizeWindow::valueChangedCb(Fl_Widget *wd, void *cbdata) {
    ((SizeWindow *)cbdata)->valueChangedCb(wd);
}

void SizeWindow::valueChangedCb(Fl_Widget *wd) {
    if (!btn_proportion_->down() || changing_proportion_) {
        return;
    }
    changing_proportion_ = true;

    Fl_Input *inp = width_;
    if (wd == height_) {
        inp = height_;
    }

    int value = 0;
    int newValue = 0;
    sscanf(inp->value(), "%d", &value);
    if (value != 0) {
       
        if (wd == height_) {
            if (proportion_to_x_) {
                newValue = value / proportion_;
            } else {
                newValue = value * proportion_;
            }
        } else {
            if (proportion_to_x_) {
                newValue = value * proportion_;
            } else {
                newValue = value / proportion_;
            }
        }

        char buffer[50] = {0,};
        sprintf(buffer, "%d", newValue);
        if (wd == height_) {
            width_->value(buffer);
        } else {
            height_->value(buffer);
        }
    }

    changing_proportion_ = false;
}

void SizeWindow::setInitialSize(int x, int y) {
    if (x != 0 && y != 0) {
        if (x >= y) {
            proportion_ = y / (float)x;
            proportion_to_x_ = true;
        } else {
            proportion_ = x / (float)y;
            proportion_to_x_ = false;
        }
    }
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
    if (width_->active()) {
        valueChangedCb(width_);
    } else {
        valueChangedCb(height_);
    }
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
