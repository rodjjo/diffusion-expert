#include <Python.h>

#include "src/python/error_handler.h"

namespace dexpert {
namespace py {

bool handle_error() {
    bool found_errors = false;

    if(PyErr_Occurred() != NULL) {
        found_errors = true;
        PyObject *pyExcType;
        PyObject *pyExcValue;
        PyObject *pyExcTraceback;
        PyErr_Fetch(&pyExcType, &pyExcValue, &pyExcTraceback);
        PyErr_NormalizeException(&pyExcType, &pyExcValue, &pyExcTraceback);

        PyObject* str_exc_type = PyObject_Repr(pyExcType);
        PyObject* pyStr = PyUnicode_AsEncodedString(str_exc_type, "utf-8", "Error ~");
        const char *strExcType =  PyBytes_AS_STRING(pyStr);

        PyObject* str_exc_value = PyObject_Repr(pyExcValue);
        PyObject* pyExcValueStr = PyUnicode_AsEncodedString(str_exc_value, "utf-8", "Error ~");
        const char *strExcValue =  PyBytes_AS_STRING(pyExcValueStr);

        PyObject* str_exc_trace = PyObject_Repr(pyExcTraceback);
        PyObject* pyExcTraceStr = PyUnicode_AsEncodedString(str_exc_trace, "utf-8", "Error ~");
        const char *strTraceValue =  PyBytes_AS_STRING(pyExcTraceStr);

        // When using PyErr_Restore() there is no need to use Py_XDECREF for these 3 pointers
        //PyErr_Restore(pyExcType, pyExcValue, pyExcTraceback);
        fprintf(stderr, "ExecType: %s\nExecValue: %s\nExecTraceback: %s\n", strExcType, strExcValue, pyExcTraceStr);

        Py_XDECREF(pyExcType);
        Py_XDECREF(pyExcValue);
        Py_XDECREF(pyExcTraceback);

        Py_XDECREF(str_exc_type);
        Py_XDECREF(pyStr);

        Py_XDECREF(str_exc_value);
        Py_XDECREF(pyExcValueStr);

        Py_XDECREF(str_exc_trace);
        Py_XDECREF(pyExcTraceStr);
    }

    
    return found_errors;
}

}  // namespace py
}  // namespace dexpert