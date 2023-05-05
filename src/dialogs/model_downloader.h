/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_DIALOGS_MODEL_DOWNLOADER_H_
#define SRC_DIALOGS_MODEL_DOWNLOADER_H_

#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Float_Input.H>

#include "src/controls/button.h"
#include "src/python/helpers.h"

namespace dexpert {

class ModelDownloader : public Fl_Window  {
 public:
    ModelDownloader(dexpert::py::model_url_list_t models_list);
    virtual ~ModelDownloader();
    static void run();

 private:
    bool ok_confirmed_ = false;
    dexpert::py::model_url_list_t models_list_;
    Fl_Choice *models_;
    std::unique_ptr<Button> btn_ok_;
    std::unique_ptr<Button> btn_cancel_;
};


void download_model_from_dialog();


}  // namespace dexpert

#endif  // SRC_DIALOGS_MODEL_DOWNLOADER_H_
