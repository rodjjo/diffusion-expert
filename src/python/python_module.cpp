
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

Dict PythonModule::newDict() { 
    return Dict(this);
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


Dict::Dict(PythonModule *m) : m_(m) {
    d_ = m->guard(PyDict_New());
}

int Dict::setString(const char *key, const char *value) {
    return PyDict_SetItemString(d_, key, m_->guard(PyUnicode_FromString(value)));
}

int Dict::setWString(const char *key, const wchar_t *value) {
    return PyDict_SetItemString(d_, key, PyUnicode_FromWideChar(value, -1));
}

int Dict::setBool(const char *key, bool value) {
   return PyDict_SetItemString(d_, key, m_->guard(PyBool_FromLong(value)));
}

int Dict::setInt(const char *key, int64_t value) {
    return PyDict_SetItemString(d_, key, m_->guard(PyLong_FromLong(value)));
}

int Dict::setFloat(const char *key, double value) {
    return PyDict_SetItemString(d_, key, m_->guard(PyFloat_FromDouble(value)));
}

int Dict::setDict(const char *key, const Dict & value) {
    return PyDict_SetItemString(d_, key, value.d_);
}

int Dict::setBytes(const char *key, const char *data, size_t len) {
    return PyDict_SetItemString(d_, key, PyBytes_FromStringAndSize((const char *)data, len));
}

int Dict::setAny(const char *key, PyObject *any) {
    return PyDict_SetItemString(d_, key,  m_->guard(any));
}

PyObject *Dict::obj() {
    return d_;
}

        
} // namespace py
} // namespace dexpert

