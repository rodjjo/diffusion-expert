#include "python/dependencies.h"
#include "windows/progress_ui.h"

namespace dfe
{
    namespace py
    {
        namespace
        {
            PYBIND11_EMBEDDED_MODULE(dexpert, m)
            {
                m.def("progress", [](size_t value, size_t max) {
                    dfe::set_progress(value, max);
                });

                m.def("progress_title", [](const char *text)
                {
                    printf("%s\n", text);
                    dfe::set_progress_title(text);
                });                
                
                m.def("progress_text", [](const char *text)
                {
                    printf("%s\n", text);
                    dfe::set_progress_text(text);
                });
                

                m.def("progress_canceled", []() {
                    return dfe::should_cancel();
                });
            }
        } // unnamed namespace

        py11::module_ dexpert_module()
        {
            py11::module_ dexp = py11::module_::import("dexpert");
            return dexp;
        }
    }
}