#pragma once

#include <string>
#include <pybind11/embed.h> 
#include <functional>

namespace py11 = pybind11;

namespace dfe
{
    namespace py
    {
        typedef std::function<void(py11::module_ &module)> callback_t;
        typedef std::function<void(bool completed)>  execute_status_cb_t;
        void execute(callback_t cb);
        std::string runPython(execute_status_cb_t status_cb);
        bool ready();
        void stopPython();
    } // namespace py
    
} // namespace dfe
