#define PY_SSIZE_T_CLEAN

#include <fstream>
#include <sstream>
#include <chrono>
#include <Python.h>

#include <Fl/Fl.H>

#include <pybind11/embed.h> 

#include "src/config/config.h"
#include "src/python/wrapper.h"
#include "src/python/raw_image.h"
#include "src/windows/progress_window.h"

namespace py11 = pybind11;

namespace dexpert {
namespace py {

namespace 
{
    std::shared_ptr<PythonMachine> machine;
    py11::module_ *main_module = NULL;

PYBIND11_EMBEDDED_MODULE(dexpert, m) {
    m.def("progress", [](size_t p, size_t m, py11::dict image) {
        image_ptr_t img;
        if (image.contains("data")) {
            img = dexpert::py::rawImageFromPyDict(image);
        }
        dexpert::set_progress(p, m, img);
    });

    m.def("progress_title", [](const char *text) {
        printf("%s\n", text);
        dexpert::set_progress_title(text);
    });

    m.def("progress_canceled", []() {
        if (dexpert::should_cancel_progress())
            return true;
        return false;
    });
}

} // unnamed namespace 


PythonMachine::PythonMachine() {
}

void PythonMachine::stop_machine() {
    terminated_ = true;
}

py11::module_ &getModule() {
    return *main_module;
}

void PythonMachine::run_machine() {
    puts("Initializing Python");

    py11::scoped_interpreter guard{};
    py11::module_ sys = py11::module_::import("sys");
    
    sys.attr("executable") = getConfig().pyExePath();
    sys.attr("_base_executable") = getConfig().pyExePath();
    py11::sequence sp = sys.attr("path").cast<py11::sequence>();
    sp.attr("append")(dexpert::getConfig().pythonStuffDir().c_str());

    py11::module_ dexp = py11::module_::import("dexpert");
    py11::module_ main = py11::module_::import("entrypoint");
    main.attr("__file__") = dexpert::getConfig().pythonMainPy();

    main_module = &main;

    while (!terminated_) {
      execute_callback_internal();
    }
}

PythonMachine::~PythonMachine() {
    terminated_ = true;
}

void PythonMachine::execute_callback_internal() {
    async_callback_t cb;
    {
        std::unique_lock<std::mutex> lk(callback_mutex_);
        cb = callback_;
    }

    if (cb) {
        try {
            cb();
        } catch(pybind11::cast_error err) {
            printf("Errored: %s\n", err.what());
            fflush(stdout);
            exit(10);
        }
        std::unique_lock<std::mutex> lk(callback_mutex_);
        callback_ = async_callback_t();
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void PythonMachine::execute_callback(async_callback_t callback) {
   replace_callback(callback);
   show_progress_window();
   wait_callback();
   hide_progress_window();
}

void PythonMachine::replace_callback(async_callback_t callback) {
    while (!callback_mutex_.try_lock()) {
        Fl::wait(0.033);
    }
    callback_ = callback;
    callback_mutex_.unlock();
}

void PythonMachine::wait_callback() {
    bool should_continue = true;
    bool locked = false;
    while (should_continue) {
        callback_mutex_.lock();
        if (!callback_) {
            should_continue = false;
        }
        callback_mutex_.unlock();
        if (should_continue) {
            Fl::wait(0.033);
        }
    }
}

std::shared_ptr<PythonMachine> get_py() {
    if (machine) {
        return machine;
    }
    machine.reset(new PythonMachine());
    return machine;
}

void py_main() {
    get_py()->run_machine();
}

void py_end() {
    get_py()->stop_machine();
}

}  // namespace py
}  // namespace dexpert
