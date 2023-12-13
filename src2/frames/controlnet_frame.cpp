
#include "components/xpm/xpm.h"

#include "frames/controlnet_frame.h"

namespace dfe
{

    namespace {
        const char *controlnet_text[controlnet_mode_max] = {
            "Disabled",
            "Scribble",
            "Canny",
            "Pose",
            "Deepth",
            "Segmentation",
            "Lineart",
            "Mangaline"
        };

        const char *controlnet_modes[controlnet_mode_max] = {
            "disabled",
            "scribble",
            "canny",
            "pose",
            "deepth",
            "segmentation",
            "lineart",
            "mangaline"
        };
    }

ControlnetFrame::ControlnetFrame(Fl_Group *parent, ImagePanel *img) {
    parent_ = parent;
    img_ = img;
    mode_ = new Fl_Choice(0, 0, 1, 1, "Controlnet mode");
    mode_->callback(combobox_cb, this);
    btnOpenMask_.reset(new Button(xpm::image(xpm::img_24x24_folder), [this] () {

    }));
    btnSaveMask_.reset(new Button(xpm::image(xpm::img_24x24_wallet), [this] () {

    }));
    btnPreprocess_.reset(new Button(xpm::image(xpm::img_24x24_pinion), [this] () {

    }));
    btnOpenMask_->tooltip("Open a pre-processed image");
    btnPreprocess_->tooltip("Pre-process the input image");
    btnSaveMask_->tooltip("Save the pre-processed image");
    
    mode_->align(FL_ALIGN_TOP_LEFT);
    for (int i = 0; i < controlnet_mode_max; i++) {
        mode_->add(controlnet_text[i]);
    }
    mode_->value(0);

    alignComponents();
    combobox_cb(mode_);
}

void ControlnetFrame::alignComponents() {
    int left = parent_->x();
    int top = parent_->y();
    int w = img_->x() - parent_->x();
    int h = parent_->h();

    mode_->resize(left + 5, top + 25, w - 10, 30);
    btnPreprocess_->position(left +5 , mode_->y() + mode_->h() + 5);
    btnPreprocess_->size(w - 10, 30);
    btnOpenMask_->position(left +5 , btnPreprocess_->y() + btnPreprocess_->h() + 5);
    btnOpenMask_->size(w - 10, 30);
    btnSaveMask_->position(left +5 , btnOpenMask_->y() + btnOpenMask_->h() + 5);
    btnSaveMask_->size(w - 10, 30);
}

ControlnetFrame::~ControlnetFrame() {
    
}

void ControlnetFrame::combobox_cb(Fl_Widget* widget, void *cbdata) {
    static_cast<ControlnetFrame *>(cbdata)->combobox_cb(widget);
}

void ControlnetFrame::combobox_cb(Fl_Widget* widget) {
    if (inside_cb_)  {
        return;
    }
    inside_cb_ = true;
    if (enabled()) {
        btnOpenMask_->show();
        btnSaveMask_->show();
        btnPreprocess_->show();
        if (parent_->visible_r()) {
            img_->show();
        }
    } else {
        btnOpenMask_->hide();
        btnSaveMask_->hide();
        btnPreprocess_->hide();
        img_->hide();
    }
    inside_cb_ = false;
}

bool ControlnetFrame::enabled() {
    return mode_->value() > 0;
}

} // namespace dfe
