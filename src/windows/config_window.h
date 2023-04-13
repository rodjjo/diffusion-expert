/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_WINDOWS_CONFIG_WINDOW_H
#define SRC_WINDOWS_CONFIG_WINDOW_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>

#include "src/controls/button.h"
#include "src/windows/modal_window.h"

namespace dexpert {

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
    Fl_Group *page_directories_;
    Fl_Tabs *tabs_;

    ModalWindow *window_ = NULL;
    std::unique_ptr<Button> btnOk_;
    std::unique_ptr<Button> btnCancel_;
};


void show_configuration();

}   // namespace dexpert

#endif  // SRC_WINDOWS_CONFIG_WINDOW_H
