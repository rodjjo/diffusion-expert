#ifndef SRC_WINDOWS_IMAGE_VIEWER_H_
#define SRC_WINDOWS_IMAGE_VIEWER_H_

#include <string>
#include <functional>
#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>

#include "src/controls/button.h"
#include "src/windows/modal_window.h"
#include "src/panels/frame_panel.h"
#include "src/python/raw_image.h"

namespace dexpert
{

class ImageViewer {
 public: 
    ImageViewer(image_ptr_t image);
    virtual ~ImageViewer();
    void show();
 private:
    void saveImage();
 private:
    image_ptr_t image_;
    ModalWindow *window_ = NULL;
    FramePanel *panel_ = NULL;
    std::unique_ptr<Button> btnSave_;
    std::unique_ptr<Button> btnClose_;
};

void view_image(image_ptr_t image);


} // namespace dexpert


#endif  // SRC_WINDOWS_IMAGE_VIEWER_H_