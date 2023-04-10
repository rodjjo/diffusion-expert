/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_GUARD_H_
#define SRC_PYTHON_GUARD_H_
#include <list>
#include <Python.h>

namespace dexpert
{
namespace py
{

class ObjGuard {
  public:
    ObjGuard(const char* module = NULL) {
        if (module) {
            module_ = PyImport_ImportModule(module);
        }
    }

    PyObject *guard(PyObject * v) {
        items_.push_back(v);
        return v;
    }

    PyObject *operator () (PyObject *v) {
        return guard(v);
    }

    ~ObjGuard() {
        Py_XDECREF(module_);
        for (auto it = items_.begin(); it != items_.end(); it++) {
            Py_XDECREF(*it);
        }
    }
    PyObject *module() {
        return module_;
    }
  private:
     PyObject* module_ = NULL;
     std::list<PyObject*> items_;
};
        
} // namespace py
} // namespace dexpert


#endif  // SRC_PYTHON_GUARD_H_
