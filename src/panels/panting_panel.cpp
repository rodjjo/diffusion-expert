#include "src/stable_diffusion/state.h"
#include "src/dialogs/utils.h"
#include "src/panels/panting_panel.h"

namespace dexpert
{

namespace {
    const char *modes_text[panting_mode_max] = {
        "Disabled",
        "Img2Img",
        "Inpaint masked",
        "Inpaint not masked",
        "Scribble",
        "Canny"
    };
}

PantingPanel::PantingPanel(int x, int y, int w, int h): Fl_Group(x, y, w, h) {
    this->begin();
    left_bar_ = new Fl_Group(0, 0, 1, 1);
    image_panel_ = new FramePanel(0, 0, 1, 1);
    image_panel_->setImageSource(image_src_self);
    left_bar_->begin();
    label_image_ = new Fl_Box(0, 0, 1, 1, "Image");
    label_mask_ = new Fl_Box(0, 0, 1, 1, "Mask");
    mode_ = new Fl_Choice(0, 0, 1, 1, "Mode");

    btnOpen_.reset(new Button(xpm::image(xpm::directory_16x16), [this] {
       openImage(); 
    }));

    btnSave_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        saveImage();
    }));

    btnOpenMask_.reset(new Button(xpm::image(xpm::directory_16x16), [this] {
        openMask();
    }));

    btnSaveMask_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        saveMask();
    }));

    left_bar_->end();

    this->end();

    btnOpen_->tooltip("Open a image");
    btnOpen_->position(1, 1);
    btnOpen_->size(48, 30);

    btnSave_->tooltip("Save the image");
    btnSave_->position(50, 1);
    btnSave_->size(48, 30);

    btnOpenMask_->tooltip("Open a mask");
    btnOpenMask_->position(1, 1);
    btnOpenMask_->size(48, 30);

    btnSaveMask_->tooltip("Save the mask");
    btnSaveMask_->position(50, 1);
    btnSaveMask_->size(48, 30);

    for (int i = 0; i < panting_mode_max; ++i) {
        mode_->add(modes_text[i]);    
    }

    mode_->value(paiting_disabled);
    mode_->align(FL_ALIGN_TOP);

    alignComponents();
}

void PantingPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

void PantingPanel::alignComponents() {
    left_bar_->resize(x(), y(), 150, h());
    image_panel_->resize(x() + left_bar_->w() + 1, y(), w() - left_bar_->w() - 2, h());

    label_image_->resize(left_bar_->x() + 1, left_bar_->y() + 1, left_bar_->w() - 2, 30);
    btnOpen_->position(left_bar_->x() + 1, label_image_->y() + label_image_->h() + 3);
    btnSave_->position(left_bar_->x() + 2 + btnOpen_->w(), btnOpen_->y());

    label_mask_->resize(left_bar_->x() + 1, btnOpen_->y() + btnOpen_->h() + 3, left_bar_->w() - 2, 30);
    btnOpenMask_->position(left_bar_->x() + 1, label_mask_->y() + label_mask_->h() + 3);
    btnSaveMask_->position(left_bar_->x() + 2 + btnOpenMask_->w(), btnOpenMask_->y());

    mode_->resize(left_bar_->x() + 1, btnOpenMask_->y() + btnOpenMask_->h() + 23, left_bar_->w() - 2, 30);
}

PantingPanel::~PantingPanel() {

}

void PantingPanel::saveImage() {
    auto img = image_panel_->getImage();
    if (img) {
        save_image_with_dialog(img);
    }
}

void PantingPanel::saveMask() {
    auto img = image_panel_->getMask();
    if (img) {
        save_image_with_dialog(img);
    }
}

void PantingPanel::openImage() {
    auto img = open_image_from_dialog();
    if (img) {
        image_panel_->setImage(img);
    }
    image_panel_->redraw();
}

void PantingPanel::openMask() {
    auto img = open_image_from_dialog();
    if (img) {
        image_panel_->setMask(img);
    }
    image_panel_->redraw();
}
    
} // namespace dexpert
