/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_WINDOWS_CONFIG_WINDOW_H
#define SRC_WINDOWS_CONFIG_WINDOW_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Choice.H>

#include "src/controls/button.h"
#include "src/windows/modal_window.h"

namespace dexpert {

typedef enum {
    controlnet_0,
    controlnet_1,
    controlnet_2,
    controlnet_3,
    controlnet_4,
    // keep controlnet_max at the end
    controlnet_max
} controlnet_counter_t;

class ConfigWindow {
 public:
    ConfigWindow();
    virtual ~ConfigWindow();
    void show();

 private:
    void align_components();
    void load_configuration();
    void save_configuration();

 private:
    Fl_Group *page_sd_;
    Fl_Tabs *tabs_;
    Fl_Check_Button *nsfw_check_;
    Fl_Choice *schedulers_;
    Fl_Choice *controlnetTabs_;

    ModalWindow *window_ = NULL;
    std::unique_ptr<Button> btnOk_;
    std::unique_ptr<Button> btnCancel_;
};


void show_configuration();

}   // namespace dexpert

#endif  // SRC_WINDOWS_CONFIG_WINDOW_H
