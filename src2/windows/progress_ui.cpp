#include <mutex>
#include <string>
#include <memory>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include "components/xpm/xpm.h"
#include "windows/progress_ui.h"

namespace dfe
{

namespace {
    progress_type current_type = progress_dependencies;
    bool progress_window_enabled = false;
    bool canceled = false;
    std::mutex progress_mutex;
    std::string progress_text;
    std::string progress_title;
    size_t progress_value = 1;
    size_t progress_max = 100;
    size_t progress_version = 0;
    std::shared_ptr<ProgressWindow> window;

    class PreventCloseWindow: public Fl_Double_Window {
        public:
            PreventCloseWindow(int ww, int hh, const char* title ) : Fl_Double_Window(ww, hh, title) {
            };
        protected:
          int handle(int event) override {
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
    };
}

ProgressWindow::ProgressWindow(progress_type ptype) {
    ptype_ = ptype;
    window_ = new PreventCloseWindow(ptype == progress_dependencies ? 900 : 550, 100, "Progress");
    if (ptype == progress_dependencies) {
        console_ = new Console(1, 1, window_->w() - 2, window_->h() - 2);
    } else {
        text_ = new Fl_Box(1, 1, window_->w() - 2, 20, "Wait...");
        progress_ = new Fl_Progress(1, text_->y() + 5 + text_->h(), window_->w() - 2, 20);
        btnCancel_.reset(new Button(
            xpm::image(xpm::img_24x24_abort),
            [this] {
                canceled = true;
                this->btnCancel_->enabled(false);
            }
        ));
        btnCancel_->size(60, 30);
        btnCancel_->position(window_->w() / 2 - btnCancel_->w() / 2, progress_->y() + progress_->h() + 5);
    }
    window_->end();
    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);
    window_->set_modal();
    Fl::add_timeout(0.01, &ProgressWindow::update, this);
}

void ProgressWindow::update(void *cbdata) {
    Fl::repeat_timeout(0.01, &ProgressWindow::update, cbdata);
    static_cast<ProgressWindow*>(cbdata)->update();
}

void ProgressWindow::update() {
    if (!progress_window_enabled) {
        return;
    }
    if (!window_->shown()) {
        window_->show(); // Keep the window open
    }
    std::unique_lock<std::mutex> lk(progress_mutex);
    if (progress_version != version_) {
        version_ = progress_version;
        this->set_progress(progress_value, progress_max);
        this->set_text(progress_text.c_str());
        this->set_title(progress_title.c_str());
    }
}

ProgressWindow::~ProgressWindow() {
    Fl::remove_timeout(&ProgressWindow::update, this);
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ProgressWindow::set_title(const char *title) {
    window_->copy_label(title);
}

void ProgressWindow::set_progress(size_t value, size_t max) {
    if (progress_) {
        if (progress_->maximum() != max) {
            progress_->maximum(max);
        }
        if (progress_->value() != value) {
            progress_->value(value);
        }
    }
}

void ProgressWindow::set_text(const char *text) {
    if (text_) {
        text_->copy_label(text);
    }
}

void init_progress_title(progress_type ptype) {
    switch (ptype)
    {
    case progress_background:
        set_progress_title("Removing the background, please wait...");
        break;
    case progress_dependencies:
        set_progress_title("Installing dependencies, please wait...");
        break;
    case progress_downloader:
        set_progress_title("Downloading file, please wait...");
        break;
    case progress_generation:
        set_progress_title("Generating the image, please wait...");
        break;
    case progress_preprocessor:
        set_progress_title("Pre-processing the image, please wait...");
        break;
    default:
        break;
    }
    progress_max = 100;
    progress_value = 0;
}

void show_progress_window(progress_type ptype,  checker_cb_t cancel_cb) {
    init_progress_title(ptype);
    ProgressWindow wnd(ptype);
    progress_window_enabled = true;
    while (!cancel_cb()) {
        Fl::wait(0.033);
    }
    progress_window_enabled = false;
}

void show_progress_window() {
    if (progress_window_enabled && !window) {
        init_progress_title(current_type);
        window.reset(new ProgressWindow(current_type));
    }
}

void enable_progress_window(progress_type ptype) {
    current_type = ptype;
    progress_window_enabled = true;
    canceled = false;
}

void hide_progress_window() {
    window.reset(); 
}

bool should_cancel() {
    return canceled;
}

void set_progress_title(const char *title) {
    std::unique_lock<std::mutex> lk(progress_mutex);
    progress_title = title;
    progress_version += 1;
}

void set_progress_text(const char *text) {
    std::unique_lock<std::mutex> lk(progress_mutex);
    progress_text = text;
    progress_version += 1;
}

void set_progress(size_t value, size_t max) {
    std::unique_lock<std::mutex> lk(progress_mutex);
    progress_value = value;
    progress_max = max;
    progress_version += 1;
}



} // namespace name
