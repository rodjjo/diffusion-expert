/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_WINDOWS_CONSOLE_VIEWER_H_
#define SRC_WINDOWS_CONSOLE_VIEWER_H_

#include <string>
#include <functional>
#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>

#include "src/console/console_tabs.h"
#include "src/controls/button.h"
#include "src/windows/modal_window.h"

namespace dexpert
{

typedef std::function<void()> callback_t;
typedef std::function<bool()> status_callback_t;

class ConsoleViewer {
  public:
    ConsoleViewer(const char *title);
    ~ConsoleViewer();
    void runInPythonContext(callback_t callback, status_callback_t status_cb);
    void showConsole(bool first_stdout);

  private:
    static void run_callback(void *cbdata);
    void run_callback();
  private:
    bool callback_completed_ = false;
    callback_t callback_;
    status_callback_t status_cb_;
    ModalWindow *window_;
    ConsoleTabs *consoles_;
    Fl_Group *bottomPanel_;
    std::unique_ptr<Button> btnClose_;
};

void showConsoles(const char* title, bool first_stdout);
void runPyShowConsole(const char* title, callback_t callback, status_callback_t status_cb);

} // namespace dexpert


#endif  // SRC_WINDOWS_CONSOLE_VIEWER_H_