#include <chrono>
#include <thread>
#include "python/dependencies.h"
#include "python/code.h"
#include "windows/progress_ui.h"
#include "windows/main_ui.h"


int main(int argc, char **argv)
{
    int result = 0;
    std::string python_error;
    /*
        It runs the gui in a secodary thread to let python to use the main thread.
        Python should use the main thread to avoid the app to hang unloading some libraries (ex. PyTorch).
    */
    std::thread gui_thread([&result, &python_error] {
        puts("Checking dependencies...");
        dfe::show_progress_window(dfe::progress_dependencies, [] () -> bool {
            return dfe::py::dependecies_ready();
        });

        while (!dfe::py::ready()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        if (!python_error.empty()) {
            result = 1;
            dfe::MainWindow::dfe_show_error(python_error.c_str());
            return;
        }

        result = dfe::MainWindow::dfe_run();
        dfe::py::stopPython();
    });

    if (!dfe::py::install_dependencies()) {
        return 1;
    }

    python_error = dfe::py::runPython([](bool completed) {
        if (completed) {
            dfe::MainWindow::dfe_hideProgress();
        } else {
            dfe::MainWindow::dfe_showProgress();
        }
    });

    if (!python_error.empty()) {
        dfe::MainWindow::dfe_stop();
    }

    gui_thread.join();
    return python_error.empty() ? result : 1;
}