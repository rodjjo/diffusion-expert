#include "src/data/event_manager.h"
#include "src/data/xpm.h"
#include "src/dialogs/common_dialogs.h"
#include "src/windows/image_viewer.h"
#include "src/stable_diffusion/state.h"

#include "src/panels/preview_panel.h"


namespace dexpert
{


PreviewPanel::PreviewPanel(PaintingPanel *painting) : Fl_Group(0, 0, 1, 1), painting_(painting) {
    begin();
    miniature_ = new ImagePanel(0, 0, 1, 1, []{});
    btnUse_.reset(new Button(xpm::image(xpm::green_pin_16x16), [this] {
        if (ask("Do you want to set this as input image ?")) {
            painting_->setImage(get_sd_state()->getResultsImage(getRow()));
            painting_->clearPasteImage();
        }
    }));
    btnView_.reset(new Button(xpm::image(xpm::lupe_16x16), [this] {
        auto img = get_sd_state()->getResultsImage(getRow());
        if (img) {
            view_image(img->duplicate());
        }
    }));
    btnRemove_.reset(new Button(xpm::image(xpm::button_delete), [this] {
        if (ask("Do you want remove this image ?")) {
            get_sd_state()->clearImage(getRow());
            updateImage();
        }
    }));
    btnScrollLeft_.reset(new Button(xpm::image(xpm::arrow_left_16x16), [this] {
        if (row_ > 0) {
            setRow(row_ - 1);
        }
    }));
    btnScrollRight_.reset(new Button(xpm::image(xpm::arrow_right_16x16), [this] {
        if (row_ + 1 < get_sd_state()->getGeneratorSize()) {
            if (Fl::event_shift() != 0) {
                get_sd_state()->generateNextVariation(getRow());
                goLastImage();
            } else {
                setRow(row_ + 1);
            }
        } else {
            if (Fl::event_shift() != 0) {
                get_sd_state()->generateNextVariation(getRow());
            } else {
                get_sd_state()->generateNextImage(getRow());
            }
            goLastImage();
        }
    }));
    lblCounter_ = new Fl_Box(0, 0, 1, 1, "0/0");
    end();
    box(FL_DOWN_BOX);

    alignComponents();

    btnUse_->tooltip("Use this image as input image (as the result)");
    btnView_->tooltip("Preview the image");
    btnRemove_->tooltip("Remove the image");
    btnScrollLeft_->tooltip("Navigate to the previous generated image");
    btnScrollRight_->tooltip("Navigate to the next generated image. (hold shift to create a variation)");

    enableControls(false);
}

PreviewPanel::~PreviewPanel() {

}

void PreviewPanel::enableControls(bool should_redraw) {
    if (get_sd_state()->getGeneratorSize() > 0) {
        miniature_->set_visible();
        btnUse_->set_visible();
        lblCounter_->set_visible();
        btnScrollLeft_->set_visible();
        btnScrollRight_->set_visible();
        btnView_->set_visible();
        btnRemove_->set_visible();
        if (should_redraw && this->visible_r()) {
            redraw();
        }
    } else {
        miniature_->hide();
        btnUse_->hide();
        lblCounter_->hide();
        btnScrollLeft_->hide();
        btnScrollRight_->hide();
        btnView_->hide();
        btnRemove_->hide();
    }
}

void PreviewPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}
    
void PreviewPanel::alignComponents() {
    miniature_->resize(x() + 3, y() + 3, w() - 6 - 26, h()- 36);
    btnUse_->size(20, 20);
    btnView_->size(20, 20);
    btnRemove_->size(20, 20);
    btnScrollLeft_->size(20, 20);
    btnScrollRight_->size(20, 20);

    btnUse_->position(x() + w() - 23, y() + 3);
    btnView_->position(x() + w() - 23, btnUse_->y() + btnUse_->h() + 2);
    btnRemove_->position(x() + w() - 23, btnView_->y() + btnView_->h() + 2);

    int navWidth = 210;
    
    btnScrollLeft_->position(x() + w() / 2 - navWidth / 2, y() + h() - 31);
    lblCounter_->resize(btnScrollLeft_->x() + btnScrollLeft_->w() + 5, btnScrollLeft_->y(), 160, 20);
    btnScrollRight_->position(lblCounter_->x() + lblCounter_->w() + 5, btnScrollLeft_->y());
}

void PreviewPanel::setRow(size_t value) {
    row_ = value;
    updateImage();
}

size_t PreviewPanel::getRow() {
    if (row_ >= get_sd_state()->getGeneratorSize() && get_sd_state()->getGeneratorSize() > 0) {
        row_ = get_sd_state()->getGeneratorSize() - 1;
    }
    return row_;
}

void PreviewPanel::goLastImage() {
    if (get_sd_state()->getGeneratorSize() < 1) {
        row_ = 0;
        return;
    }
    row_ = get_sd_state()->getGeneratorSize() - 1;
    updateImage();
}

void PreviewPanel::updateImage() {
    auto img = get_sd_state()->getResultsImage(getRow());
    if (!img) {
        miniature_->close();
    } else {
        int sz = w();
        if (h() > sz) {
            sz = h();
        }
        if (sz < 100) {
            sz = 100;
        }
        miniature_->setLayerImage(image_type_image, img->duplicate());
        miniature_->zoomFit();
    }

    enableControls(true);
    char buffer[100] = "";
    sprintf(buffer, "%d of %d",  (int)row_ + 1, (int)get_sd_state()->getGeneratorSize());
    lblCounter_->copy_label(buffer);
}

    
} // namespace dexpert
