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

    class PythonModule;

    class Dict
    {
    public:
      Dict(PythonModule *m);

      int setString(const char *key, const char *value);
      int setWString(const char *key, const wchar_t *value);
      int setBool(const char *key, bool value);
      int setInt(const char *key, int64_t value);
      int setFloat(const char *key, double value);
      int setDict(const char *key, const Dict &value);
      int setBytes(const char *key, const char *data, size_t len);
      int setAny(const char *key, PyObject *any);
      PyObject *obj();

    private:
      PyObject *d_;
      PythonModule *m_;
    };

    class PythonModule
    {
    public:
      PythonModule(const PythonModule &) = delete;
      PythonModule &operator=(const PythonModule &) = delete;
      PythonModule(const char *module = NULL);
      PyObject *guard(PyObject *v);
      PyObject *get_function(const char *name);
      virtual ~PythonModule();
      PyObject *module();
      Dict newDict();

    public:
      /*
      template <class... Args>
      PyObject *call(const char *name, const char *format, Args... args)
      {
        PyObject *result = NULL;
        PyObject *fun = get_function(name);
        if (fun)
        {
          result = guard(PyObject_CallFunction(fun, format, args...));
        }
        return result;
      } */

      PyObject *call(const char *name, int argCount, ...);

    private:
      PyObject *module_ = NULL;
      PyGILState_STATE gil_state_;
      std::list<PyObject *> items_;
    };

  } // namespace py
} // namespace dexpert

#endif // SRC_PYTHON_PYTHON_MODULE_H_
