#include "src/data/xpm.h"
#include "src/dialogs/common_dialogs.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"
#include "src/windows/config_window.h"

namespace dexpert {

ConfigWindow::ConfigWindow() {
    window_ = new ModalWindow(0, 0, 640, 480, "Configurations");
    window_->begin();
    tabs_ = new Fl_Tabs(0, 0, 1, 1);
    tabs_->begin();
    page_sd_ = new Fl_Group(0, 0, 1, 1, "Stable Diffusion");
    nsfw_check_ = new Fl_Check_Button(0, 0, 1, 1, "Filter NSFW");
    schedulers_ = new Fl_Choice(0, 0, 1, 1, "Scheduler");
    tabs_->end();
    btnOk_.reset(new Button(xpm::image(xpm::button_ok_16x16), [this] {
        save_configuration();
    }));
    btnCancel_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
        window_->hide();        
    }));
    window_->end();

    window_->enableExitShortcuts();

    btnOk_->tooltip("Save the configuration");
    btnCancel_->tooltip("Save the configuration");
    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);

    schedulers_->align(FL_ALIGN_TOP_LEFT);

    align_components();
    load_configuration();
}

ConfigWindow::~ConfigWindow() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ConfigWindow::align_components() {
    tabs_->resize(0, 0, window_->w(), window_->h() - 50);

    page_sd_->resize(tabs_->x(), tabs_->y() + 20, tabs_->w(), tabs_->h() - 22);
    int left = tabs_->x() + 5;
    int top = tabs_->y() + 50;
    int height = 30;
    schedulers_->resize(left, top, 200, height);
    nsfw_check_->resize(left + schedulers_->w() + 5, top, 200, height);
    btnOk_->position(window_->w() - 215, window_->h() - 40);
    btnOk_->size(100, 30);
    btnCancel_->position(btnOk_->x() + btnOk_->w() + 2, btnOk_->y());
    btnCancel_->size(100, 30);

    schedulers_->add("PNDMScheduler");
    schedulers_->add("DDIMScheduler");
    schedulers_->add("LMSDiscreteScheduler");
    schedulers_->value(0);
}

void ConfigWindow::load_configuration() {
    nsfw_check_->value(getConfig().getSafeFilter());
    int index = schedulers_->find_index(getConfig().getScheduler());
    if (index >= 0) {
        schedulers_->value(index);
    }
}

void ConfigWindow::save_configuration() {
    window_->hide();

    getConfig().setSafeFilter(nsfw_check_->value() == 1);
    if (schedulers_->value() >= 0) {
        getConfig().setScheduler(schedulers_->text(schedulers_->value()));
    } else {
        getConfig().setScheduler(schedulers_->text(0));
    }

    const char *msg;
    bool success = false;
    dexpert::py::get_py()->execute_callback(dexpert::py::configure_stable_diffusion([&success, &msg] (bool status, const char *error) {
        msg = error;
        success = status;
    }));
    if (msg) {
        dexpert::show_error(msg);
    } else {
        dexpert::show_error("Some configurations may require to restart the application");
    }
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
