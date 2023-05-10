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
#include "src/windows/splash_screen.h"

#include "src/stable_diffusion/state.h"
#include "src/stable_diffusion/generator_txt2img.h"
#include "src/stable_diffusion/generator_img2img.h"

/*
namespace dexpert {
    
    void test_generators() {
        std::shared_ptr<GeneratorBase> g;
        g.reset(new GeneratorTxt2Image(
            "An astronaut riding a horse at he moon",
            "drawing",
            get_sd_state()->getSdModelPath("model.safetensors"),
            controlnet_list_t(),
            100,
            512,
            512,
            50,
            7.5,
            0.1,
            false,
            false
        ));

        if (!get_sd_state()->generatorAdd(g)) {
            show_error(get_sd_state()->lastError());
            showConsoles("Ops", true);
        }
    }
}
*/

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
        It runs the gui in a secodary thread to let python to use the main thread.
        Python should use the main thread to avoid the app to hang unloading some libraries (ex. PyTorch).
    */
    std::thread gui_thread([&result] {
        Fl::scheme("gtk+");
        /*
        for (int i = 0; i < 100; i++) {
            dexpert::test_generators();
        }
        */

        bool have_deeps = false;
        const char *msg = NULL;
        dexpert::py::get_py()->execute_callback(
            dexpert::py::check_have_deps([&have_deeps, &msg] (bool status, const char *error) {
                msg = error;
                have_deeps = status;
            })
        );

        if (msg) {
            dexpert::py::py_end();    
            dexpert::show_error(msg);
            dexpert::showConsoles("Unexpected Error", false);
            result = 1;
            return;
        }

        if (!have_deeps && !install_deps()) {
            dexpert::py::py_end();    
            result = 2;
            return;
        }

        dexpert::py::get_py()->setDepsOk();
        dexpert::wait_python();

        bool success = false;
        dexpert::py::get_py()->execute_callback(dexpert::py::configure_stable_diffusion([&success, &msg] (bool status, const char *error) {
            msg = error;
            success = status;
        }));

        if (!success) {
            dexpert::py::py_end();    
            dexpert::show_error(msg);
            dexpert::showConsoles("Unexpected Error", false);
            result = 1;
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
