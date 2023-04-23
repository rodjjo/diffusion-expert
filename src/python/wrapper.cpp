#define PY_SSIZE_T_CLEAN

#include <chrono>
#include <Python.h>

#include <Fl/Fl.H>

#include "src/config/config.h"
#include "src/python/wrapper.h"
#include "src/python/python_stuff.h"
#include "src/python/raw_image.h"
#include "src/python/python_module.h"
#include "src/windows/progress_window.h"


namespace dexpert {
namespace py {

namespace {
std::shared_ptr<PythonMachine> machine;

PyObject* report_progress(PyObject *self, PyObject *args)
{
    int p = 0, m = 100;
    PyObject *image;
    if(!PyArg_ParseTuple(args, "iiO", &p, &m, &image))
        return NULL;

    set_progress(p, m, rawImageFromPyDict(image));

    Py_RETURN_NONE;
}


PyObject* report_task(PyObject *self, PyObject *args)
{
    const char *text = NULL;
    if(!PyArg_ParseTuple(args, "s", &text))
        return NULL;
    if (text) {
        printf("%s\n", text);
        set_progress_title(text);
    }
    Py_RETURN_NONE;
}


PyObject* should_cancel(PyObject *self, PyObject *args)
{
    if (should_cancel_progress())
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}



PyMethodDef dexpertMethods[] = {
    {"progress", report_progress, METH_VARARGS, "Report the image generation progress and preview"},
    {"progress_title", report_task, METH_VARARGS, "Set the current task title"},
    {"progress_canceled", should_cancel, METH_NOARGS, "Check if the user wants to cancel"},
    {NULL, NULL, 0, NULL}
};


struct PyModuleDef cModDexpertProgress =
{
    PyModuleDef_HEAD_INIT,
    "dexpert", 
    "The diffusion expert module",          
    -1,          
    dexpertMethods
};


PyMODINIT_FUNC PyInit_dexpertModule(void)
{
    PyObject *mod = PyModule_Create(&cModDexpertProgress);
    return  mod;
}

}  // unnamed namespaces


PythonMachine::PythonMachine() {
}

void PythonMachine::stop_machine() {
    terminated_ = true;
}


void PythonMachine::run_machine() {
    wchar_t *program = Py_DecodeLocale("diffusion_expert", NULL);
    wchar_t *argv[1] = {(wchar_t *)getConfig().pythonMainPy().c_str()};

    Py_SetProgramName(program);
    Py_SetPythonHome(getConfig().executableDir().c_str());

    puts("Initializing Python");

    Py_Initialize();
    PySys_SetArgvEx(1, argv, 1);
    {  // module context
        PythonModule module;
    
        PyObject *msys = module.guard(PyImport_ImportModule("sys"));
        PyObject* main = module.guard(PyImport_AddModule("__main__")); // hold the main module
        
        module.guard(PyInit_dexpertModule());

        PyObject* dexpert_mod = PyImport_AddModule("dexpert");
        PyModule_AddFunctions(dexpert_mod, dexpertMethods);


        PyObject *pyString = module.guard(PyUnicode_FromWideChar(getConfig().pyExePath().c_str(), -1));
        PyObject_SetAttrString(msys, "executable", pyString);
        PyObject_SetAttrString(msys, "_base_executable", pyString);

        PyObject *pyLibPath = module.guard(PyUnicode_FromWideChar(getConfig().librariesDir().c_str(), -1));
        PyObject *pyAuto111Path = module.guard(PyUnicode_FromWideChar(getConfig().librariesDir().c_str(), -1));
        PyObject *pathList = module.guard(PyObject_GetAttrString(msys, "path"));
        
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
