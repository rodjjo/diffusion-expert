#include "src/data/xpm.h"
#include "src/python/wrapper.h"

#include "src/windows/console_viewer.h"


namespace dexpert
{

ConsoleViewer::ConsoleViewer(const char *title) {
    window_ = new ModalWindow(0, 0, 1024, 480, title);
    consoles_ = new ConsoleTabs(0, 0, window_->w(), window_->h() - 60);
    btnClose_.reset(new Button(xpm::image(xpm::button_stop), [this] {
        window_->hide();
    }));
    btnClose_->tooltip("Close the window");
    btnClose_->position(window_->w() / 2 - 50, window_->h() - 50);
    btnClose_->size(100, 30);
    window_->end();
    window_->set_modal();
    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);
    consoles_->hideShowConsoleButtons();
}

ConsoleViewer::~ConsoleViewer() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ConsoleViewer::runInPythonContext(callback_t callback, status_callback_t status_cb) {
    callback_ = callback;
    status_cb_ = status_cb;
    btnClose_->enabled(false);
    Fl::add_timeout(0.101, &ConsoleViewer::run_callback, this);
    showConsole(true);
}

void ConsoleViewer::run_callback(void *cbdata) {
    ((ConsoleViewer *) cbdata)->run_callback();
}

void ConsoleViewer::run_callback() {
    puts("Console execute py callback");
    py::get_py()->execute_callback(callback_);
    btnClose_->enabled(true);

    if (status_cb_) {
        if (status_cb_()) {
            window_->hide();
        }
    }
}

void ConsoleViewer::showConsole(bool first_stdout) {
    if (first_stdout)
        consoles_->goStdout();
    else 
        consoles_->goStderr();

    window_->show();
    while (true) {
        if (callback_) {
            if (!btnClose_->enabled() && !window_->visible_r()) {
                window_->show();
            }
        }
        if (!window_->visible_r()) {
            break;
        }
        Fl::wait();
    }
}

void showConsoles(const char* title, bool first_stdout) {
    std::unique_ptr<ConsoleViewer> cv(new ConsoleViewer(title));
    cv->showConsole(true);
}

void runPyShowConsole(const char* title, callback_t callback, status_callback_t status_cb) {
    std::unique_ptr<ConsoleViewer> cv(new ConsoleViewer(title));
    cv->runInPythonContext(callback, status_cb);
}

}  // namespace dexpert