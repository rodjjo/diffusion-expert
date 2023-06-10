#ifndef SRC_WINDOWS_IMAGE_VIEWER_H_
#define SRC_WINDOWS_IMAGE_VIEWER_H_

#include <string>
#include <functional>
#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>

#include "src/panels/painting_panel.h"
#include "src/controls/image_panel.h"
#include "src/controls/button.h"
#include "src/windows/modal_window.h"
#include "src/python/raw_image.h"


namespace dexpert
{

class ImageViewer {
 public: 
    ImageViewer(image_ptr_t image, bool copy_mode);
    virtual ~ImageViewer();
    void show();
    void set_src(image_ptr_t image);
    image_ptr_t get_merged();
    bool confirmed();

 private:
    void saveImage();
    static void page_changed(Fl_Widget* element, void *data);
    static void brushSelected(Fl_Widget *widget, void *cbdata);
 private:
    image_ptr_t image_;
    image_ptr_t src_;
    bool confirmed_ = false;
    bool copy_mode_ = false;
    ModalWindow *window_ = NULL;
    ImagePanel *panel_ = NULL;
    ImagePanel *panel2_ = NULL;
    Fl_Group *page1_ = NULL;
    Fl_Group *page2_ = NULL;
    Fl_Tabs *tabs_ = NULL;
    Fl_Choice *brushes_;
    std::unique_ptr<Button> btnSave_;
    std::unique_ptr<Button> btnClose_;
};

void view_image(image_ptr_t image);
image_ptr_t copy_inpaint(image_ptr_t src_image, image_ptr_t result_image);

} // namespace dexpert


#endif  // SRC_WINDOWS_IMAGE_VIEWER_H_