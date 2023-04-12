#ifndef SRC_WINDOWS_PROGRESS_WINDOW_H_
#define SRC_WINDOWS_PROGRESS_WINDOW_H_

#include <string>
#include <functional>
#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Box.H>

#include "src/controls/button.h"
#include "src/python/raw_image.h"
#include "src/panels/frame_panel.h"
#include "src/windows/modal_window.h"

namespace dexpert
{

class ProgressWindow {
 public: 
    ProgressWindow();
    virtual ~ProgressWindow();
    void show();
    void hide();

 protected:
    void update();
    static void update(void *cbdata);

 private:
    ModalWindow *window_ = NULL;
    FramePanel *preview_;
    Fl_Progress *progress_;
    Fl_Box* title_;
    std::unique_ptr<Button> btnCancel_;
    
};

void set_progress_title(const char *title);
void set_progress(int progress, int max, image_ptr_t preview);
bool should_cancel_progress();
void enable_progress_window();
void show_progress_window();
void hide_progress_window();

} // namespace dexpert


#endif  // SRC_WINDOWS_PROGRESS_WINDOW_H_