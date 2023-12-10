#include <mutex>
#include <thread>
#include <chrono>

#include <Fl/Fl.H>

#include "python/code.h"
#include "python/module.h"
#include "misc/utils.h"

namespace dfe
{
    namespace py
    {
        namespace
        {
            bool running = true;
            bool python_ready = false;
            execute_status_cb_t on_status;
            std::mutex callback_mutex;
            callback_t current_callback;
            py11::module_ *main_module = NULL;
        }

        void replace_callback(callback_t callback)
        {
            while (!callback_mutex.try_lock())
            {
                Fl::wait(0.033);
            }
            current_callback = callback;
            callback_mutex.unlock();
        }

        void wait_callback()
        {
            bool should_continue = true;
            bool locked = false;
            while (should_continue)
            {
                callback_mutex.lock();
                if (!current_callback)
                {
                    should_continue = false;
                }
                callback_mutex.unlock();
                if (should_continue)
                {
                    Fl::wait(0.033);
                }
            }
        }

        void execute(callback_t cb)
        {
            replace_callback(cb);
            on_status(false);
            wait_callback();
            on_status(true);
        }

        void execute_next_cb()
        {
            callback_t cb;
            { // locking area
                std::unique_lock<std::mutex> lk(callback_mutex);
                cb = current_callback;
            } // locking area

            if (!cb) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return;
            }

            try {
                cb(*main_module);
            } catch(pybind11::cast_error err) {
                printf("Errored: %s\n", err.what());
                fflush(stdout);
            }

            { // locking area
                std::unique_lock<std::mutex> lk(callback_mutex);
                current_callback = callback_t();
            } // locking area
        }

        bool init_python() {
            return false;
        }

        std::string runPython(execute_status_cb_t status_cb)
        {
            try {
                python_ready = false;

                py11::scoped_interpreter guard{};
                py11::module_ sys = py11::module_::import("sys");

                sys.attr("executable") = pythonExecutablePath();
                sys.attr("_base_executable") = pythonExecutablePath();
                py11::sequence sp = sys.attr("path").cast<py11::sequence>();
                sp.attr("append")(sourcesDirectory().c_str());
                sp.attr("append")(comfyuiDirectory().c_str());
                py11::module_ dxpert(dexpert_module());
                py11::module_ main = py11::module_::import("diffusion_expert");
                main.attr("__file__") = sourcesDirectory() + L"diffusion_expert.py";

                main_module = &main;
                on_status = status_cb;

                puts("Python started...");
                running = true;
                python_ready = true;
                while (running)
                {
                    execute_next_cb();
                }
            } catch (std::exception e) {
                python_ready = true;
                return std::string("errored!");
            }

            puts("Python stopped...");
            return std::string();
        }

        bool ready() {
            return python_ready;
        }

        void stopPython()
        {
            puts("Stopping python...");
            running = false;
        }
    } // namespace py

} // namespace dfe
