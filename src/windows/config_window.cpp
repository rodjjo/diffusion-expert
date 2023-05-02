#include "src/data/xpm.h"
#include "src/dialogs/common_dialogs.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"
#include "src/config/config.h"
#include "src/windows/config_window.h"

namespace dexpert {

namespace {
    const char *controlnet_choices[controlnet_max] = {
        "Only for the input image",
        "Controlnet +1",
        "Controlnet +2",
        "Controlnet +3",
        "Controlnet +4"
    };

    const char *gpgan_archs[gfpgan_arch_count] = {
        "clean",
        "RestoreFormer"
    };
}

ConfigWindow::ConfigWindow() {
    window_ = new ModalWindow(0, 0, 640, 480, "Configurations");
    window_->begin();
    tabs_ = new Fl_Tabs(0, 0, 1, 1);

    tabs_->begin();
    
    page_sd_ = new Fl_Group(0, 0, 1, 1, "Stable Diffusion");
    page_sd_->begin();
    nsfw_check_ = new Fl_Check_Button(0, 0, 1, 1, "Filter NSFW");
    float16_check_ = new Fl_Check_Button(0, 0, 1, 1, "Use float16");
    gpu_check_ = new Fl_Check_Button(0, 0, 1, 1, "Use GPU (CUDA)");
    schedulers_ = new Fl_Choice(0, 0, 1, 1, "Scheduler");
    controlnetCount_ = new Fl_Choice(0, 0, 1, 1, "Controlnet");
    page_sd_->end();    
    
    page_upscalers_ = new Fl_Group(0, 0, 1, 1, "Upscalers");
    page_upscalers_->begin();
    gfpgan_arch_ = new Fl_Choice(0, 0, 1, 1, "Face restoration arch");
    gfpgan_only_center_faces_ = new Fl_Check_Button(0, 0, 1, 1, "Only centered faces");
    gfpgan_weight_ = new Fl_Float_Input(0, 0, 1, 1, "Face restoration weigth");
    page_upscalers_->end();

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
    controlnetCount_->align(FL_ALIGN_TOP_LEFT);
    gfpgan_arch_->align(FL_ALIGN_TOP_LEFT);
    gfpgan_weight_->align(FL_ALIGN_TOP_LEFT);

    schedulers_->add("UniPCMultistepScheduler");
    schedulers_->add("PNDMScheduler");
    schedulers_->add("DDIMScheduler");
    schedulers_->add("LMSDiscreteScheduler");
    schedulers_->value(0);
    
    for (int i = 0; i < controlnet_max; i++) {
        controlnetCount_->add(controlnet_choices[i]);
    }
    controlnetCount_->value(0);
    for (int i = 0; i < gfpgan_arch_count; ++i) {
        gfpgan_arch_->add(gpgan_archs[i]);
    }
    gfpgan_arch_->value(0);
    gfpgan_weight_->value("0.5");

    align_components();
    load_configuration();
}

ConfigWindow::~ConfigWindow() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ConfigWindow::align_components() {
    tabs_->resize(0, 0, window_->w(), window_->h() - 50);
    page_sd_->resize(tabs_->x(), tabs_->y() + 30, tabs_->w(), tabs_->h() - 22);
    page_upscalers_->resize(tabs_->x(), tabs_->y() + 30, tabs_->w(), tabs_->h() - 22);
    int left = tabs_->x() + 10;
    int top = tabs_->y() + 55;
    int height = 30;
    
    // TAB: STABLE DIFFUSION
    schedulers_->resize(left, top, 200, height);
    nsfw_check_->resize(left + schedulers_->w() + 5, top, 200, height);
    controlnetCount_->resize(nsfw_check_->x() + nsfw_check_->w() + 5, top, 200, height);
    float16_check_->resize(left, top + 5 + controlnetCount_->h(), 200, height);
    gpu_check_->resize(float16_check_->x() + float16_check_->w() + 5, float16_check_->y(), 200, height);
    btnOk_->position(window_->w() - 215, window_->h() - 40);
    btnOk_->size(100, 30);
    btnCancel_->position(btnOk_->x() + btnOk_->w() + 2, btnOk_->y());
    btnCancel_->size(100, 30);

    // TAB: UPSCALERS
    gfpgan_arch_->resize(left, top, 200, height);
    gfpgan_only_center_faces_->resize(gfpgan_arch_->x() + gfpgan_arch_->w() + 5, top, 200, height);
    gfpgan_weight_->resize(gfpgan_only_center_faces_->x() + gfpgan_only_center_faces_->w() + 5, top, 200, height);
}

void ConfigWindow::load_configuration() {
    auto &c = getConfig();
    nsfw_check_->value(c.getSafeFilter());
    float16_check_->value(c.getUseFloat16());
    gpu_check_->value(c.getUseGPU());

    int index = gfpgan_arch_->find_index(c.gfpgan_get_arch());
    if (index >= 0) {
        gfpgan_arch_->value(index);
    }
    gfpgan_only_center_faces_->value(c.gfpgan_get_only_center_face());
    char temp[32] = {0, };
    sprintf(temp, "%0.3f", c.gfpgan_get_weight());
    gfpgan_weight_->value(temp);

    index = schedulers_->find_index(c.getScheduler());
    if (index >= 0) {
        schedulers_->value(index);
    }
    controlnetCount_->value(c.getControlnetCount());
}

void ConfigWindow::save_configuration() {
    window_->hide();
    auto &c = getConfig();
    c.setSafeFilter(nsfw_check_->value() == 1);
    c.setUseFloat16(float16_check_->value() == 1);
    c.setUseGPU(gpu_check_->value() == 1);

    c.gfpgan_set_arch(gfpgan_arch_->text(gfpgan_arch_->value()));
    c.gfpgan_set_only_center_face(gfpgan_only_center_faces_->value() == 1);
    
    float result = 0.5;
    sscanf(gfpgan_weight_->value(), "%f", &result);
    if (result < 0)
        result = 0.0;
    else if (result > 1.0)
        result = 1.0;
    c.gfpgan_set_weight(result);

    if (schedulers_->value() >= 0) {
        c.setScheduler(schedulers_->text(schedulers_->value()));
    } else {
        c.setScheduler(schedulers_->text(0));
    }
    c.setControlnetCount(controlnetCount_->value());
    c.save();

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
