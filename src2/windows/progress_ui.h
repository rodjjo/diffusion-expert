#pragma once

#include <functional>
#include <FL/Fl_Window.H>

#include "terminal/console.h"

namespace dfe
{

typedef enum {
    progress_dependencies,
    progress_downloader,
    progress_generation,
    progress_upscaler,
    progress_background,
    progress_preprocessor
} progress_type;

typedef std::function<bool()> checker_cb_t;

class ProgressWindow {
  public:
    ProgressWindow(progress_type ptype);
    virtual ~ProgressWindow();
  private:
    static void update(void *cbdata);
    void update();
    void set_title(const char *title);
    void set_progress(size_t value, size_t max);
    void set_text(const char *text);

  private:
    Fl_Window *window_;
    Console *console_ = NULL;
    size_t version_ = 0;
    progress_type ptype_;
};

void show_progress_window(progress_type ptype, checker_cb_t cancel_cb);
void show_progress_window();
void enable_progress_window(progress_type ptype);
void hide_progress_window();
bool should_cancel();
void set_progress_title(const char *title);
void set_progress_text(const char *text);
void set_progress(size_t value, size_t max);


    
} // namespace dfe
