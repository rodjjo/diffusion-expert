/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_WRAPPER_H_
#define SRC_PYTHON_WRAPPER_H_

#include <functional>
#include <mutex>
#include <thread>
#include <memory>

#include <pybind11/embed.h> 

namespace py11 = pybind11;

namespace dexpert {
namespace py {

typedef std::function<void()> async_callback_t;
class PyMachineSingleton;
class PythonMachine;
std::shared_ptr<PythonMachine> get_py();
void py_main();
void py_end();

py11::module_ &getModule();

class PythonMachine {
 private:
   friend std::shared_ptr<PythonMachine> get_py(); 
   PythonMachine();
   friend void py_main();
   void run_machine();
   friend void py_end();
   void stop_machine();
 public:
    PythonMachine (const PythonMachine &) = delete;
    PythonMachine & operator = (const PythonMachine &) = delete;
    virtual ~PythonMachine();
    void execute_callback(async_callback_t callback);
    // void execute(async_callback_t cb);

 private:
    void execute_callback_internal();
    void replace_callback(async_callback_t callback);
    void wait_callback();

 private:
   bool terminated_ = false;
   std::mutex callback_mutex_;
   async_callback_t callback_;
};

}  // namespace py
}  // namespace dexpert

#endif  // SRC_PYTHON_WRAPPER_H_
