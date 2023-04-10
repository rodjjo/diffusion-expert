#define PY_SSIZE_T_CLEAN

#include <chrono>
#include <Python.h>

#include <Fl/Fl.H>

#include "src/config/config.h"
#include "src/python/wrapper.h"
#include "src/python/python_stuff.h"
#include "src/python/guard.h"

namespace dexpert {
namespace py {

namespace {
std::shared_ptr<PythonMachine> machine;
}  // unnamed namespaces


PythonMachine::PythonMachine() {
    /*
    thread_.reset(new std::thread([this]{
        run_machine();
    }));
    */
}

void PythonMachine::stop_machine() {
    terminated_ = true;
}


void PythonMachine::run_machine() {
    wchar_t *program = Py_DecodeLocale("dexpert", NULL);
    wchar_t *argv[1] = {(wchar_t *)getConfig().pythonMainPy().c_str()};

    Py_SetProgramName(program);
    Py_SetPythonHome(getConfig().executableDir().c_str());

    puts("Initializing Python");

    Py_Initialize();
    PySys_SetArgvEx(1, argv, 1);

    {  // guard context
        ObjGuard guard;
    
        PyObject *msys = guard(PyImport_ImportModule("sys"));
        PyObject* main = guard(PyImport_AddModule("__main__")); // hold the main module

        PyObject *pyString = guard(PyUnicode_FromWideChar(getConfig().pyExePath().c_str(), -1));
        PyObject_SetAttrString(msys, "executable", pyString);
        PyObject_SetAttrString(msys, "_base_executable", pyString);

        PyObject *pyLibPath = guard(PyUnicode_FromWideChar(getConfig().librariesDir().c_str(), -1));
        PyObject *pyAuto111Path = guard(PyUnicode_FromWideChar(getConfig().librariesDir().c_str(), -1));
        PyObject *pathList = guard(PyObject_GetAttrString(msys, "path"));
        
        PyList_Append(pathList, pyLibPath);
        PyList_Append(pathList, pyAuto111Path);

        initialize_python_stuff(main);

        while (!terminated_) {
            execute_callback_internal();
        }
    }

    puts("Finalizing Python");
    if (Py_FinalizeEx() < 0) {
        puts("Thre is an error in python machine");
    }

    PyMem_RawFree(program);
}

PythonMachine::~PythonMachine() {
    terminated_ = true;
    if (thread_) {
        thread_->join();
    } 
}

void PythonMachine::execute_callback_internal() {
    async_callback_t cb;
    {
        std::unique_lock<std::mutex> lk(callback_mutex_);
        cb = callback_;
    }

    if (cb) {
        cb();
        std::unique_lock<std::mutex> lk(callback_mutex_);
        callback_ = async_callback_t();
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void PythonMachine::execute_callback(async_callback_t callback) {
   replace_callback(callback);
   wait_callback();
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
