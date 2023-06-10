#include "src/data/xpm.h"
#include "src/windows/image_viewer.h"
#include "src/stable_diffusion/state.h"
#include "src/dialogs/utils.h"

namespace dexpert
{
namespace {
    const char *brush_captions[brush_size_count] = {
        "Disabled",
        "1 Pixel",
        "2 Pixels",
        "4 Pixels",
        "8 Pixels",
        "16 Pixels",
        "32 Pixels",
        "64 Pixels",
        "128 Pixels"
    };

    const uint8_t brushes_sizes[brush_size_count] = {
        0, 1, 2, 4, 8, 16, 32, 64, 128
    };
}

ImageViewer::ImageViewer(image_ptr_t image, bool copy_mode) {
    copy_mode_ = copy_mode;
    image_ = image;
    window_ = new ModalWindow(0, 0, 1024, 860, copy_mode ? "Copy the area under the mask" : "Preview the image");
    window_->enableExitShortcuts();
    window_->begin();
    if (copy_mode) {
        tabs_ = new Fl_Tabs(0, 0, 1024, 800);
        tabs_->begin();
        page1_ = new Fl_Group(0, 0, 1024, 780, "Original");
        page1_->begin();
    }
    panel_ = new ImagePanel(0, 0, 1024, 800, [] {});
    if (copy_mode) {
        page1_->end();
        page2_ = new Fl_Group(0, 0, 1024, 780, "Result");
        page2_->begin();
        panel2_ = new ImagePanel(0, 0, 1024, 780, [] {});
        page2_->end();
        tabs_->end();

        panel2_->size(panel2_->w(), 780);
        panel_->size(panel2_->w(), 780);

        page1_->size(panel_->w(), panel_->h());
        page2_->size(panel_->w(), panel_->h());

        tabs_->callback(ImageViewer::page_changed, this);

        brushes_ = new Fl_Choice(5, window_->h() - 45, 200, 20, "Brush size");
        for (int i = 0; i < brush_size_count; ++i) {
            brushes_->add(brush_captions[i]);
        }

        brushes_->align(FL_ALIGN_TOP);
        brushes_->value(5);
        brushes_->callback(brushSelected, this);
    }
    
    btnClose_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
        window_->hide();
    }));
    
    btnSave_.reset(new Button(xpm::image(copy_mode ? xpm::button_ok_16x16 : xpm::save_16x16 ), [this] {
        saveImage();
    }));

    btnClose_->tooltip("Close the window");
    btnClose_->position(window_->w() / 2 - 50, window_->h() - 50);
    btnClose_->size(100, 30);
    
    btnSave_->tooltip(copy_mode ? "Accept the selected area" : "Save the image");
    btnSave_->position(10, window_->h() - 50);
    btnSave_->size(100, 30);

    if (copy_mode_) {
        int btn_w = (btnClose_->w() + 5 + btnSave_->w());
        btn_w =  (window_->w() / 2) - btn_w / 2;
        btnSave_->position(btn_w, btnSave_->y());
        btnClose_->position(btn_w + btnSave_->w() + 5, btnClose_->y());
    }

    window_->end();

    window_->position(Fl::w() / 2 - window_->w() / 2, Fl::h() / 2 - window_->h() / 2);
    panel_->setLayerImage(image_type_image, image);

    if (copy_mode_) {
        panel_->setEditType(edit_type_mask);
    }
}

ImageViewer::~ImageViewer() {
    Fl::delete_widget(window_);
    Fl::do_widget_deletion();
}

void ImageViewer::page_changed(Fl_Widget* element, void *data) {
    auto v = (ImageViewer *) data;
    v->panel2_->setLayerImage(image_type_image, v->get_merged());
}

void ImageViewer::brushSelected(Fl_Widget *widget, void *cbdata) {
    auto v = (ImageViewer *) cbdata;
    v->panel_->setBrushSize(brushes_sizes[v->brushes_->value()]);
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
    if (copy_mode_) {
        confirmed_ = true;
        window_->hide();
    } else {
        save_image_with_dialog(image_);
    }
}

void ImageViewer::set_src(image_ptr_t image) {
    src_ = image;
    panel_->setLayerImage(image_type_mask, dexpert::py::newImage(src_->w(), src_->h(), true));
    panel_->setEditType(edit_type_mask);
    panel_->setLayerVisible(image_type_mask, true);
    panel_->setTool(image_tool_brush);
    panel_->setBrushSize(16);
}

image_ptr_t ImageViewer::get_merged() {
    image_ptr_t r;
    if (src_.get() != NULL) {
        r = src_->duplicate();
        auto mask = panel_->getLayerImage(image_type_mask);
        if (mask) {
            r->pasteAt(0, 0, mask->removeAlpha().get(), image_.get());
        }
    }
    return r;
}

bool ImageViewer::confirmed() {
    return confirmed_;
}

void view_image(image_ptr_t image) {
    ImageViewer viewer(image, false);
    viewer.show();
}



image_ptr_t copy_inpaint(image_ptr_t src_image, image_ptr_t result_image) {
    ImageViewer viewer(result_image, true);
    viewer.set_src(src_image);
    viewer.show();
    if (viewer.confirmed()) {
        return viewer.get_merged();
    }
    return src_image;
}



} // namespace dexpert
