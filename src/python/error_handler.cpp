#include <Python.h>

#include "src/python/error_handler.h"
#include "src/python/python_module.h"

namespace dexpert {
namespace py {

bool handle_error() {
    if(PyErr_Occurred() != NULL) {
        PyErr_Print();
        return true;
    }
    return false;
}

}  // namespace py
}  // namespace dexpert