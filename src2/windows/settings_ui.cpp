#include "components/xpm/xpm.h"
#include "windows/settings_ui.h"
#include "misc/dialogs.h"

#include "misc/config.h"

namespace dfe
{

SettingsWindow::SettingsWindow() : Fl_Window(Fl::w() / 2 - 640 / 2, Fl::h() / 2 - 480 / 2, 640, 480, "Configurations") {
    auto wnd_ = this;
    wnd_->begin();

    tabs_ = new Fl_Tabs(0, 0, 1, 1);
    tabs_->begin();
    page_params_ = new Fl_Group(0, 0, 1, 1, "Parameters");
    filter_nsfw_ = new Fl_Check_Button(0, 0, 1, 1, "Filter NSFW");
    use_float16_ = new Fl_Check_Button(0, 0, 1, 1, "Use float16");
    private_mode_ = new Fl_Check_Button(0, 0, 1, 1, "Private mode");
    keep_models_ = new Fl_Check_Button(0, 0, 1, 1, "Keep models in the memory");
    page_params_->end();

    tabs_->begin();
    
    page_directories_ = new Fl_Group(0, 0, 1, 1, "Directories");
    add_model_dir_ = new Fl_Input(1, 1, 1, 1, "Additional stable diffusion model dir");
    add_lora_dir_ = new Fl_Input(1, 1, 1, 1, "Additional lora model dir");
    add_emb_dir_ = new Fl_Input(1, 1, 1, 1, "Additional embedding dir");
    page_directories_->end();

    wnd_->begin();
    btnOk_.reset(new Button(xpm::image(xpm::img_24x24_ok), [this] {
        this->save_settings();
        this->hide();
    }));
    btnCancel_.reset(new Button(xpm::image(xpm::img_24x24_abort), [this] {
        this->hide();        
    }));

    wnd_->end();
    wnd_->set_modal();

    btnOk_->tooltip("Save the configuration");
    btnCancel_->tooltip("Discart the changes");
    keep_models_->tooltip("Move the model to RAM when it exchanges between inpaint and normal model");

    add_model_dir_->align(FL_ALIGN_TOP_LEFT);
    add_lora_dir_->align(FL_ALIGN_TOP_LEFT);
    add_emb_dir_->align(FL_ALIGN_TOP_LEFT);

    alignComponents();
}

SettingsWindow::~SettingsWindow() {
}

void SettingsWindow::save_settings() {
    auto cfg = get_config();
    cfg->add_emb_dir(add_emb_dir_->value());
    cfg->add_lora_dir(add_lora_dir_->value());
    cfg->add_model_dir(add_model_dir_->value());
    cfg->filter_nsfw(filter_nsfw_->value() == 1);
    cfg->use_float16(use_float16_->value() == 1);
    cfg->private_mode(private_mode_->value() == 1);
    cfg->save();
}

void SettingsWindow::load_settings() {
    auto cfg = get_config();
    cfg->load();
    add_emb_dir_->value(cfg->add_emb_dir().c_str());
    add_lora_dir_->value(cfg->add_lora_dir().c_str());
    add_model_dir_->value(cfg->add_model_dir().c_str());
    filter_nsfw_->value((int) cfg->filter_nsfw());
    use_float16_->value((int) cfg->use_float16());
    private_mode_->value((int) cfg->private_mode());
}

int SettingsWindow::handle(int event) {
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
    return Fl_Window::handle(event);
}

void SettingsWindow::alignComponents() {
    ((Fl_Widget *) tabs_)->resize(0, 0, this->w(), this->h() - 50);
    
    page_params_->resize(tabs_->x(), tabs_->y() + 30, tabs_->w(), tabs_->h() - 22);
    page_directories_->resize(tabs_->x(), tabs_->y() + 30, tabs_->w(), tabs_->h() - 22);

    int left = tabs_->x() + 10;
    int top = tabs_->y() + 55;
    int height = 30;

    btnOk_->position(this->w() - 215, this->h() - 40);
    btnOk_->size(100, 30);
    btnCancel_->position(btnOk_->x() + btnOk_->w() + 2, btnOk_->y());
    btnCancel_->size(100, 30);

    add_model_dir_->resize(left, top, page_directories_->w() - 20, height);
    add_lora_dir_->resize(left, add_model_dir_->y() + add_model_dir_->h() + 20, page_directories_->w() - 20, height);
    add_emb_dir_->resize(left, add_lora_dir_->y() + add_lora_dir_->h() + 20, page_directories_->w() - 20, height);

    filter_nsfw_->resize(left, top, 200, height);
    use_float16_->resize(left + filter_nsfw_->w() + 5, top, 200, height);
    private_mode_->resize(use_float16_->x() + use_float16_->w() + 5, top, 200, height);
    keep_models_->resize(private_mode_->x() + private_mode_->w() + 5, top, 200, height);
}

void edit_settings() {
    auto window = new SettingsWindow();
    window->load_settings();
    window->show();
    while (true) {
        if (!window->visible_r()) {
            break;
        }
        Fl::wait();
    }
    Fl::delete_widget(window);
    Fl::do_widget_deletion();
}
    
} // namespace dfe
