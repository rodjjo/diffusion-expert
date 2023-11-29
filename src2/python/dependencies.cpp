#include "python/dependencies.h"
#include "python/module.h"
#include "misc/utils.h"


namespace dfe
{
    namespace py
    {
        namespace {
            bool ready = false;
        }

        bool install_dependencies() {
            ready = false;
            py11::scoped_interpreter guard{};
            py11::module_ sys = py11::module_::import("sys");
            sys.attr("executable") = pythonExecutablePath();
            sys.attr("_base_executable") = pythonExecutablePath();
            py11::sequence sp = sys.attr("path").cast<py11::sequence>();
            sp.attr("append")(sourcesDirectory().c_str());
            py11::module_ dxpert(std::move(dexpert_module()));
            py11::module_ deeps = py11::module_::import("dependencies");
            deeps.attr("install")();
            ready = true;
            return true; // TODO(Rodrigo): Check dependency installation failure
        } 

        bool dependecies_ready() {
            return ready;
        }
    }  // namespace py
} // namespace dfe
