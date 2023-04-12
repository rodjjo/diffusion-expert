#include <memory>
#include <mutex>
#include <string>
#include <atomic>
#include "src/python/raw_image.h"
#include "src/windows/progress_window.h"


namespace dexpert
{
namespace {
    bool progress_enabled = false;
    bool progress_canceled = false;
    std::unique_ptr<ProgressWindow> prog_win;
    std::string progress_label;

    std::mutex mtx_progress;
    image_ptr_t image;

    int current_progress = 0;
    int max_progress = 100;
}

ProgressWindow::ProgressWindow() {
    window_ = new ModalWindow(0, 0, 640, 480, "Please wait");
    window_->begin();
    preview_ = new FramePanel(image_ptr_t(), 0, 0, window_->w(), window_->h() - 85);
    title_ = new Fl_Box(5, preview_->h() + 3, window_->w() - 10, 20, "Wait");
    progress_ = new Fl_Progress(5, title_->y() + title_->h() + 3, window_->w() - 10, 15);
    btnCancel_.reset(new Button(xpm::image(xpm::button_stop), [this] {
        progress_canceled = true;
        btnCancel_->enabled(false);
    }));

    window_->end();
    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);

    progress_->maximum(100);
    progress_->value(0);

    btnCancel_->tooltip("Cancel the operation");
    btnCancel_->position(window_->w() / 2 - 50, window_->h() - 40);
    btnCancel_->size(100, 30);


    Fl::add_timeout(0.01, &ProgressWindow::update, this);
}

ProgressWindow::~ProgressWindow() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}
 
void ProgressWindow::show() {
    window_->show();
}

void ProgressWindow::hide() {
    window_->hide();
}

void ProgressWindow::update(void *cbdata) {
    if (prog_win.get() == cbdata) {
        prog_win->update();
        Fl::repeat_timeout(0.33, ProgressWindow::update, cbdata); // retrigger timeout
    }
}

void ProgressWindow::update() {
    std::unique_lock<std::mutex> lk(mtx_progress);
    progress_->maximum(max_progress);
    progress_->value(current_progress);
    if (image) {
        preview_->setImage(image);
        image.reset();
        preview_->redraw();
    }
    if (!progress_label.empty()) {
        title_->copy_label(progress_label.c_str());
        progress_label.clear();
    }
}

void set_progress_title(const char *title) {
    std::unique_lock<std::mutex> lk(mtx_progress);
    if (title)
        progress_label = title;
}

void set_progress(int progress, int max, image_ptr_t preview) {
    std::unique_lock<std::mutex> lk(mtx_progress);
    current_progress = progress;
    max_progress = max;
    image = preview;
}

bool should_cancel_progress() {
    return progress_canceled;
}

void enable_progress_window() {
    progress_enabled = true;
    progress_canceled = false;
 }

void show_progress_window() {
    if (prog_win) 
        return;
    if (!progress_enabled)
        return;
    current_progress = 0;
    max_progress = 100;
    prog_win.reset(new ProgressWindow());
    prog_win->show();
    progress_enabled = false;
}

void hide_progress_window() {
    if (prog_win) {
        prog_win->hide();
        prog_win.reset();
    }
}


} // namespace dexpert
