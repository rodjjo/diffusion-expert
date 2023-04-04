#include <fstream>
#include <sstream>
#include "src/python/python_stuff.h"
#include "src/python/error_handler.h"
#include "src/config/config.h"


namespace dexpert {
namespace py {
namespace {
    bool initialized = false;
    bool initialized_result = false;
}

bool initialize_python_stuff(PyObject* main) {
    if (initialized) {
        return initialized_result;
    }
    initialized = true;
    std::ifstream iofile(dexpert::getConfig().pythonMainPy().c_str());
    std::stringstream buffer;
    buffer << iofile.rdbuf();

    PyObject* globalDictionary = PyModule_GetDict(main);
    // PyObject* localDictionary = PyDict_New();
    PyObject* pyString = PyUnicode_FromWideChar(dexpert::getConfig().pythonMainPy().c_str(), -1);

    PyDict_SetItemString(globalDictionary, "__file__", pyString);

    PyObject* result = PyRun_String(buffer.str().c_str(), Py_file_input, globalDictionary, globalDictionary);
    
    handle_error();

    Py_XDECREF(globalDictionary);
    // Py_XDECREF(localDictionary);
    Py_XDECREF(result);
    Py_XDECREF(pyString);

    initialized_result = true;
    return false;
}

}  // namespace py
}  // namespace

