#include "src/data/event_manager.h"
#include "src/data/xpm.h"
#include "src/stable_diffusion/state.h"
#include "src/panels/preview_panel.h"

namespace dexpert
{


PreviewPanel::PreviewPanel(int x, int y, int w, int h, bool main_preview) : Fl_Group(x, y, w, h) {
    begin();
    miniature_ = new ImagePanel(0, 0, 1, 1, []{});
    btnUse_.reset(new Button(xpm::image(xpm::green_pin_16x16), [this] {
        trigger_event(this, preview_event_use);
    }));
    btnPrevious_.reset(new Button(xpm::image(xpm::arrow_up_16x16), [this] {
        trigger_event(this, preview_event_previous);
    }));
    btnNext_.reset(new Button(xpm::image(xpm::arrow_down_16x16), [this] {
        trigger_event(this, preview_event_next);
    }));
    btnPreviousVar_.reset(new Button(xpm::image(xpm::arrow_left_16x16), [this] {
        trigger_event(this, preview_event_previous_var);
    }));
    btnNextVar_.reset(new Button(xpm::image(xpm::arrow_right_16x16), [this] {
        trigger_event(this, preview_event_next_var);
    }));
    btnView_.reset(new Button(xpm::image(xpm::lupe_16x16), [this] {
        trigger_event(this, preview_event_view);
    }));
    btnRemove_.reset(new Button(xpm::image(xpm::button_delete), [this] {
        trigger_event(this, preview_event_remove);
    }));
    end();
    box(FL_DOWN_BOX);

    alignComponents();
    main_preview_ = main_preview;
    
    btnPrevious_->hide();
    btnPreviousVar_->hide();
    btnNextVar_->hide();
    btnNext_->hide();
    
    btnUse_->hide();
    btnView_->hide();
    btnRemove_->hide();

    btnPrevious_->tooltip("Generate the previous image");
    btnPreviousVar_->tooltip("Generate the previous variation");
    btnNextVar_->tooltip("Generate the next variation");
    btnNext_->tooltip("Generate the next image");
    btnUse_->tooltip("Use this image as input image (as the result)");
    btnView_->tooltip("Preview the image");
    btnRemove_->tooltip("Remove the image");
}

PreviewPanel::~PreviewPanel() {

}

void PreviewPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}
    
void PreviewPanel::alignComponents() {
    miniature_->resize(x() + 3, y() + 3, w() - 6 - 26, h()- 6);
    btnPrevious_->size(20, 20);
    btnPreviousVar_->size(20, 20);
    btnNextVar_->size(20, 20);
    btnNext_->size(20, 20);
    btnUse_->size(20, 20);
    btnView_->size(20, 20);
    btnRemove_->size(20, 20);

    btnPrevious_->position(x() + w() - 23, y() + 3 );
    btnNext_->position(x() + w() - 23,  btnPrevious_->y() + btnPrevious_->h() + 2);

    btnUse_->position(x() + w() - 23, btnNext_->y() + btnNext_->h() + 12);
    btnView_->position(x() + w() - 23, btnUse_->y() + btnUse_->h() + 2);
    btnRemove_->position(x() + w() - 23, btnView_->y() + btnView_->h() + 2);

    btnPreviousVar_->position(x() + w() - 23, btnRemove_->y() + btnRemove_->h() + 12);
    btnNextVar_->position(x() + w() - 23, btnPreviousVar_->y() + btnPreviousVar_->h() + 2);
    
}

void PreviewPanel::setCol(size_t value) {
    col_ = value;
}

size_t PreviewPanel::getCol() {
    return col_;
}

void PreviewPanel::setRow(size_t value) {
    row_ = value;
}

size_t PreviewPanel::getRow() {
    return row_;
}

void PreviewPanel::updateImage() {
    auto img = get_sd_state()->getResultsImage(getRow(), getCol());
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

    if (!main_preview_ || !miniature_->getReferenceImage()) {
        btnPrevious_->hide();
        btnPreviousVar_->hide();
        btnNextVar_->hide();
        btnNext_->hide();
    } else {
        btnPrevious_->show();
        btnPreviousVar_->show();
        btnNextVar_->show();
        btnNext_->show();
    }
    if (miniature_->getReferenceImage()) {
        btnUse_->show();
        btnView_->show();
        btnRemove_->show();
    } else {
        btnUse_->hide();
        btnView_->hide();
        btnRemove_->hide();
    }
}

    
} // namespace dexpert
