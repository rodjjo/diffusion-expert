/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_PYTHON_MODULE_H_
#define SRC_PYTHON_PYTHON_MODULE_H_

#include <stdarg.h>
#include <list>
#include <Python.h>

namespace dexpert
{
namespace py
{

class PythonModule {
  public:
    PythonModule (const PythonModule &) = delete;
    PythonModule & operator = (const PythonModule &) = delete;
    PythonModule(const char* module = NULL);
    PyObject *guard(PyObject * v);
    PyObject *get_function(const char *name);
    virtual ~PythonModule();
    PyObject *module();
    PyObject *newDict();
    int dictSetString(PyObject *d, const char *key, const char *value);
    int dictSetBool(PyObject *d, const char *key, bool value);
    int dictSetInt(PyObject *d, const char *key, int64_t value);
    int dictSetFloat(PyObject *d, const char *key, double value);
    int dictSetDict(PyObject *d, const char *key, PyObject *value);
  public:
    template <class ... Args>
    PyObject *operator () (const char *name, const char *format, Args ... args) {
      PyObject *result = NULL;
      PyObject *fun = get_function(name);
      if (fun) {
          result = guard(PyObject_CallFunction(fun, format, args...));
      }
      return result;
    }                    

  private:
     PyObject* module_ = NULL;
     PyGILState_STATE gil_state_;
     std::list<PyObject*> items_;
};
        
} // namespace py
} // namespace dexpert


#endif  // SRC_PYTHON_PYTHON_MODULE_H_
