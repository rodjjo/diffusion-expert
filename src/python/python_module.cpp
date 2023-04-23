
#include <string>
#include <map>

#include "src/python/python_module.h"

namespace dexpert
{
namespace py
{

namespace {
    std::map<std::string, PyObject*> modules_cache;
    std::map<PyObject *, std::map<std::string, PyObject *> > module_functions;
}

PythonModule::PythonModule(const char* module) {
    gil_state_ = PyGILState_Ensure();
    if (module) {
        auto it = modules_cache.find(module);
        if (it != modules_cache.end()) {
            module_ = it->second;
        } else {
            module_ = PyImport_ImportModule(module);
            Py_XINCREF(module_);
            modules_cache[module] = module_;
        }
    }
}

PythonModule::~PythonModule() {
    for (auto it = items_.begin(); it != items_.end(); it++) {
        Py_XDECREF(*it);
    }
    PyGILState_Release(gil_state_);
}

PyObject *PythonModule::guard(PyObject * v) {
    if (v) {
        items_.push_back(v);
    }
    return v;
}

PyObject *PythonModule::newDict() { 
    return guard(PyDict_New());
}


int PythonModule::dictSetString(PyObject *d, const char *key, const char *value) {
    return PyDict_SetItemString(d, key, guard(PyUnicode_FromString(value)));
}

int PythonModule::dictSetBool(PyObject *d, const char *key, bool value) {
   return PyDict_SetItemString(d, key, guard(PyBool_FromLong(value)));
}

int PythonModule::dictSetInt(PyObject *d, const char *key, int64_t value) {
    return PyDict_SetItemString(d, key, guard(PyLong_FromLong(value)));
}

int PythonModule::dictSetFloat(PyObject *d, const char *key, double value) {
    return PyDict_SetItemString(d, key, guard(PyFloat_FromDouble(value)));
}

int PythonModule::dictSetDict(PyObject *d, const char *key, PyObject *value) {
    return PyDict_SetItemString(d, key, value);
}

PyObject *PythonModule::get_function(const char *name) {
    auto im = module_functions.find(module_);
    if (im != module_functions.end()) {
        auto ifunc = im->second.find(name);
        if (ifunc != im->second.end()) {
            return ifunc->second;
        }
    }
    PyObject *fn = PyObject_GetAttrString(module_, name);
    if (fn) {
        Py_XINCREF(fn);
        module_functions[module_][name] = fn;
    }
    return fn;
}


PyObject *PythonModule::module() {
    return module_;
}

        
} // namespace py
} // namespace dexpert

