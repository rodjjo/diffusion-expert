#pragma once

#include <pybind11/embed.h> 

namespace py11 = pybind11;

namespace dfe
{
    namespace py
    {
        bool install_dependencies();
        bool dependecies_ready();
    }  // namespace py
} // namespace dfe
