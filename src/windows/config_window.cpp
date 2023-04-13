#include "src/data/xpm.h"
#include "src/windows/config_window.h"

namespace dexpert {

ConfigWindow::ConfigWindow() {
    window_ = new ModalWindow(0, 0, 640, 480, "Configurations");
    window_->begin();
    tabs_ = new Fl_Tabs(0, 0, 1, 1);
    tabs_->begin();
    page_directories_ = new Fl_Group(0, 0, 1, 1, "Directories");
    tabs_->end();
    btnOk_.reset(new Button(xpm::image(xpm::button_stop), [this] {
        save_configuration();
    }));
    btnCancel_.reset(new Button(xpm::image(xpm::button_stop), [this] {
        window_->hide();        
    }));
    window_->end();

    window_->enableExitShortcuts();

    btnOk_->tooltip("Save the configuration");

    btnCancel_->tooltip("Save the configuration");

    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);

    align_components();
    load_configuration();
}

ConfigWindow::~ConfigWindow() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ConfigWindow::align_components() {
    tabs_->resize(0, 0, window_->w(), window_->h() - 50);

    page_directories_->resize(tabs_->x(), tabs_->y() + 20, tabs_->w(), tabs_->h() - 22);

    btnOk_->position(window_->w() - 215, window_->h() - 40);
    btnOk_->size(100, 30);
    btnCancel_->position(btnOk_->x() + btnOk_->w() + 2, btnOk_->y());
    btnCancel_->size(100, 30);
}

void ConfigWindow::load_configuration() {

}

void ConfigWindow::save_configuration() {
    window_->hide();
}

void ConfigWindow::show() {
    window_->show();
    while (true) {
        if (!window_->visible_r()) {
            break;
        }
        Fl::wait();
    }
}

void show_configuration() {
    ConfigWindow cfg;
    cfg.show();
}

}  // namespace dexpert
