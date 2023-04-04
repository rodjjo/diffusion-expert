#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "src/python/wrapper.h"
#include "src/python/helpers.h"
#include "src/dialogs/common_dialogs.h"
#include "src/windows/console_viewer.h"
#include "src/windows/main_window.h"


bool install_deps() {
    bool success = false;
    std::string message = "Unexpected error. Callback did not run!";
    dexpert::runPyShowConsole("Installing Python dependencies", [&success, &message] {
        auto installer = dexpert::py::install_deps([&success, &message] (bool st, const char *msg) {
            success = st;
            if (msg) {
                message = msg;
            }
        });
        installer();
    }, [&success, &message] () -> bool {
        if (!success) {
            dexpert::show_error(message.c_str());
        }
        return success;
    });

    return success;
}


int main(int argc, char **argv)
{
    int result = 0;
    /*
        It run the gui in a secodary thread to let python to use the main thread.
        Python should use the main thread to avoid the app to hang unloading some libraries (ex. PyTorch).
    */
    std::thread gui_thread([&result] {
        Fl::scheme("gtk+");

        bool have_deps = false;
        const char *msg = NULL;
        dexpert::py::get_py()->execute_callback(
            dexpert::py::check_have_deps([&have_deps, &msg] (bool status, const char *error) {
                msg = error;
                have_deps = status;
            })
        );

        if (msg) {
            dexpert::py::py_end();    
            dexpert::show_error(msg);
            result = 1;
            return;
        }

        if (!have_deps && !install_deps()) {
            dexpert::py::py_end();    
            result = 2;
            return;
        }

        auto w = new dexpert::MainWindow(); // fltk deletes the object after we run it.
        result = w->run();
        dexpert::py::py_end();
    });

    dexpert::py::py_main();
    gui_thread.join();
    return result;
}
