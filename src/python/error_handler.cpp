#include <Python.h>

#include "src/python/error_handler.h"
#include "src/python/guard.h"

namespace dexpert {
namespace py {

bool handle_error() {
    bool found_errors = false;

    if(PyErr_Occurred() != NULL) {
        found_errors = true;
        PyObject *pyExcType;
        PyObject *pyExcValue;
        PyObject *pyExcTraceback;
        ObjGuard guard;
        PyErr_Fetch(&pyExcType, &pyExcValue, &pyExcTraceback);
        PyErr_NormalizeException(&pyExcType, &pyExcValue, &pyExcTraceback);
        
        guard(pyExcType);
        guard(pyExcValue);
        guard(pyExcTraceback);

        PyObject* str_exc_type = guard(PyObject_Repr(pyExcType));
        PyObject* pyStr = guard(PyUnicode_AsEncodedString(str_exc_type, "utf-8", "Error ~"));
        const char *strExcType =  PyBytes_AS_STRING(pyStr);

        PyObject* str_exc_value = guard(PyObject_Repr(pyExcValue));
        PyObject* pyExcValueStr = guard(PyUnicode_AsEncodedString(str_exc_value, "utf-8", "Error ~"));
        const char *strExcValue =  PyBytes_AS_STRING(pyExcValueStr);

        PyObject* str_exc_trace = guard(PyObject_Repr(pyExcTraceback));
        PyObject* pyExcTraceStr = guard(PyUnicode_AsEncodedString(str_exc_trace, "utf-8", "Error ~"));
        const char *strTraceValue =  PyBytes_AS_STRING(pyExcTraceStr);

        // When using PyErr_Restore() there is no need to use Py_XDECREF for these 3 pointers
        //PyErr_Restore(pyExcType, pyExcValue, pyExcTraceback);
        fprintf(stderr, "ExecType: %s\nExecValue: %s\nExecTraceback: %s\n", strExcType, strExcValue, pyExcTraceStr);
    }

    
    return found_errors;
}

}  // namespace py
}  // namespace dexpert