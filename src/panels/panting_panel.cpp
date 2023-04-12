#include "src/panels/panting_panel.h"

namespace dexpert
{

PantingPanel::PantingPanel(int x, int y, int w, int h): Fl_Group(x, y, w, h) {
    this->begin();
    left_bar_ = new Fl_Group(0, 0, 1, 1);
    image_panel_ = new FramePanel (0, 0, 1, 1);

    left_bar_->begin();
    btnOpen_.reset(new Button(xpm::image(xpm::directory_16x16), [this] {
        
    }));
    btnSave_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        
    }));
    left_bar_->end();

    this->end();

    btnOpen_->tooltip("Open a image");
    btnOpen_->position(1, 1);
    btnOpen_->size(48, 30);

    btnSave_->tooltip("Save the image");
    btnSave_->position(50, 1);
    btnSave_->size(48, 30);


    alignComponents();
}

void PantingPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

void PantingPanel::alignComponents() {
    left_bar_->resize(x(), y(), 100, h());
    image_panel_->resize(x() + left_bar_->w() + 1, y(), w() - left_bar_->w() - 2, h());
    btnOpen_->position(left_bar_->x() + 1, left_bar_->y() + 1);
    btnSave_->position(left_bar_->x() + 2 + btnOpen_->w(), left_bar_->y() + 1);
}

PantingPanel::~PantingPanel() {

}
    
} // namespace dexpert
