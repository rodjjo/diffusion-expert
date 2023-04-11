#include "src/panels/panting_panel.h"

namespace dexpert
{

PantingPanel::PantingPanel(int x, int y, int w, int h): Fl_Group(x, y, w, h) {
    this->begin();
    left_bar_ = new Fl_Group(0, 0, 1, 1);
    image_panel_ = new FramePanel (0, 0, 1, 1);
    this->end();
    alignComponents();
}

void PantingPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

void PantingPanel::alignComponents() {
    left_bar_->resize(x(), y(), 100, h());
    image_panel_->resize(x() + left_bar_->w() + 1, y(), w() - left_bar_->w() - 2, h());
}

PantingPanel::~PantingPanel() {

}
    
} // namespace dexpert
