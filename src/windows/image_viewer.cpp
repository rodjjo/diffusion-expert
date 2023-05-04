#include "src/windows/image_viewer.h"
#include "src/stable_diffusion/state.h"
#include "src/dialogs/utils.h"

namespace dexpert
{
namespace {
    std::string current_open_input_dir_; 
}

ImageViewer::ImageViewer(image_ptr_t image) {
    image_ = image;
    window_ = new ModalWindow(0, 0, 1024, 860, "Preview the image");
    window_->enableExitShortcuts();
    window_->begin();
    panel_ = new ImagePanel(0, 0, 1024, 800, [] {});
    
    btnClose_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
        window_->hide();
    }));
    
    btnSave_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        saveImage();
    }));

    btnClose_->tooltip("Close the window");
    btnClose_->position(window_->w() / 2 - 50, window_->h() - 50);
    btnClose_->size(100, 30);
    
    btnSave_->tooltip("Save the image");
    btnSave_->position(10, window_->h() - 50);
    btnSave_->size(100, 30);

    window_->end();

    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);
    panel_->setLayerImage(image_type_image, image);
}

ImageViewer::~ImageViewer() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ImageViewer::show() {
    window_->show();
    while (true) {
        if (!window_->visible_r()) {
            break;
        }
        Fl::wait();
    }
}

void ImageViewer::saveImage() {
    save_image_with_dialog(image_);
}

void view_image(image_ptr_t image) {
    ImageViewer viewer(image);
    viewer.show();
}



} // namespace dexpert
