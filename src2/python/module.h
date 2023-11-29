#pragma once

#include <pybind11/embed.h> 
#include <functional>

namespace py11 = pybind11;

namespace dfe
{
    namespace py
    {
        py11::module_ dexpert_module();
    } // namespace py
} // namespace dfe
