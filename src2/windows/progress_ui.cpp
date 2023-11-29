#include <mutex>
#include <string>
#include <memory>
#include <FL/Fl.H>

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
}

ProgressWindow::ProgressWindow(progress_type ptype) {
    ptype_ = ptype;
    window_ = new Fl_Window(900, 300, "Progress");
    if (ptype == progress_dependencies) {
        console_ = new Console(1, 1, window_->w() - 2, window_->h() - 2);
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

}

void ProgressWindow::set_text(const char *text) {

}

void show_progress_window(progress_type ptype,  checker_cb_t cancel_cb) {
    set_progress_title("Installing dependencies, please wait...");
    ProgressWindow wnd(ptype);
    progress_window_enabled = true;
    while (!cancel_cb()) {
        Fl::wait(0.033);
    }
    progress_window_enabled = false;
}

void show_progress_window() {
    if (progress_window_enabled && !window) {
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
