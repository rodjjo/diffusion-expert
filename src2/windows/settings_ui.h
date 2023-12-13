#pragma once

#include <memory>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Tabs.H>
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Check_Button.H>

#include "components/button.h"

namespace dfe
{
    
class SettingsWindow: public Fl_Window {
public:
    SettingsWindow();
    ~SettingsWindow();
    int handle(int event);
    void save_settings();
    void load_settings();

private:
    void alignComponents();

private:
    Fl_Tabs *tabs_;
    Fl_Group *page_params_;
    Fl_Group *page_directories_;
    Fl_Input *add_model_dir_;
    Fl_Input *add_lora_dir_;
    Fl_Input *add_emb_dir_;
    Fl_Check_Button *filter_nsfw_;
    Fl_Check_Button *use_float16_;
    Fl_Check_Button *private_mode_;
    Fl_Check_Button *keep_models_;

    std::unique_ptr<Button> btnOk_;
    std::unique_ptr<Button> btnCancel_;
};

void edit_settings();

} // namespace dfe
