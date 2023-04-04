/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_STUFF_H_
#define SRC_PYTHON_STUFF_H_

#include <Python.h>

namespace dexpert {
namespace py {

bool initialize_python_stuff(PyObject* main);

}  // namespace py
}  // namespace

#endif  // SRC_PYTHON_STUFF_H_
