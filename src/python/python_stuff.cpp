#include <fstream>
#include <sstream>
#include "src/python/python_stuff.h"
#include "src/python/error_handler.h"
#include "src/python/python_module.h"
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

    PythonModule module;

    PyObject* globalDictionary = module.guard(PyModule_GetDict(main));
    // PyObject* localDictionary = guard(PyDict_New());
    PyObject* pyString = module.guard(PyUnicode_FromWideChar(dexpert::getConfig().pythonMainPy().c_str(), -1));

    PyDict_SetItemString(globalDictionary, "__file__", pyString);

    PyObject* result = module.guard(PyRun_String(buffer.str().c_str(), Py_file_input, globalDictionary, globalDictionary));
    
    handle_error();

    initialized_result = true;
    return false;
}

}  // namespace py
}  // namespace

