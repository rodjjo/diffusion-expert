#include "src/data/xpm.h"
#include "src/python/wrapper.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"
#include "src/dialogs/model_downloader.h"

namespace dexpert
{

ModelDownloader::ModelDownloader(dexpert::py::model_url_list_t models_list) : Fl_Window(0, 0, 300, 100, "Download models") {
    models_list_ = models_list;
    
    this->position(Fl::w()/ 2 - this->w() / 2,  Fl::h() / 2 - this->h() / 2);

    models_ = new Fl_Choice(5, 25, this->w() - 10, 30, "Model to download");
    models_->align(FL_ALIGN_TOP_LEFT);
   
    btn_ok_.reset(new Button(xpm::image(xpm::button_ok_16x16), [this] {
        ok_confirmed_ = true;
        this->hide();
    }));
    btn_cancel_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
        this->hide();
    }));

    btn_ok_->tooltip("Confirm and close.");
    btn_cancel_->tooltip("Cancel and close.");
    btn_cancel_->size(60, 30);
    btn_ok_->size(60, 30);
    btn_cancel_->position(w() - 5 - btn_cancel_->w(), h() - 5 - btn_cancel_->h());
    btn_ok_->position(btn_cancel_->x() - 5 - btn_ok_->w(), btn_cancel_->y());

    this->set_modal();
    this->show();

    for (auto it = models_list_.cbegin(); it != models_list_.cend(); it++) {
        models_->add(it->displayName.c_str());
    }
    models_->value(0);
}

ModelDownloader::~ModelDownloader() {

}

void ModelDownloader::run() {
    dexpert::py::model_url_list_t model_list;
    
    const char *error = NULL;
    dexpert::py::get_py()->execute_callback(
        dexpert::py::model_urls([&model_list, &error] (bool, const char *msg, dexpert::py::model_url_list_t list ) {
            model_list = list;
            error = msg;
        })
    );

    if (error) {
        show_error(error);
        return;
    }

    if (model_list.size() < 1) {
        show_error("No models to download");
        return;
    }
    
    ModelDownloader *wnd = new ModelDownloader(model_list);

    wnd->show();
    while (wnd->shown()) {
        Fl::wait(0.01);
    }
    
    if (wnd->ok_confirmed_) {
        auto it = model_list.begin();
        std::advance(it, wnd->models_->value());

        dexpert::py::get_py()->execute_callback(
            dexpert::py::download_model(it->url.c_str(), it->filename.c_str(), [&model_list, &error] (bool, const char *msg) {
                error = msg;
            })
         );

        if (error) {
            show_error(error);
        } else {
            get_sd_state()->reloadSdModelList();
        }
    }

    Fl::delete_widget(wnd);
    Fl::do_widget_deletion();
}

void download_model_from_dialog() {
    ModelDownloader::run();
}

} // namespace dexpert
