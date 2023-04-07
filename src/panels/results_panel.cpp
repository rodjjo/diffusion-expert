#include "src/panels/results_panel.h"

namespace dexpert
{

ResultsPanel::ResultsPanel(int x, int y, int w, int h): Fl_Group(x, y, w, h) {
    this->begin();
    for (int i = 0; i < 4; i++) {
        std::list<FramePanel *> row;
        for (int j = 0; j < 5; j++) {
            row.push_back(new FramePanel(0, 0, 1, 1));
            (*row.rbegin())->setGridLocation(j, i);
            (*row.rbegin())->enableGrid();
            (*row.rbegin())->enableCache();
            (*row.rbegin())->setImageSource(image_src_results);
        }
        miniatures_.push_back(row);
    }
    this->end();
    updatePanels();
}

ResultsPanel::~ResultsPanel() {

}

void ResultsPanel::updatePanels() {
    for (auto it = miniatures_.begin(); it != miniatures_.end(); it++) {
        for (auto it2 = it->begin(); it2 != it->end(); it2++) {
            // it2->hide();
        }
    }
    alignComponents();
}

void ResultsPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    updatePanels();
};

void ResultsPanel::alignComponents() {
    int max_row = 0;
    int max_col = 0;
    for (auto it = miniatures_.begin(); it != miniatures_.end(); it++) {
        for (auto it2 = it->begin(); it2 != it->end(); it2++) {
            FramePanel *frame = *it2;
            if (frame->shown()) {
                if (frame->gridCol() > max_col)
                    max_col = frame->gridCol();
                if (frame->gridRow() > max_row)
                    max_row = frame->gridRow();
            }
        }
    }
    max_row += 1;
    max_col += 1;
    int max_w = w() - 10;
    int max_h = h() - 10;
    int minature_w = (max_w - (5 * max_col) - 5) / max_col;
    int minature_h = (max_h - (5 * max_row) - 5) / max_row;
    int sx = x() + 5;
    int xx = sx;
    int yy = y() + 5;
    for (auto it = miniatures_.begin(); it != miniatures_.end(); it++) {
        for (auto it2 = it->begin(); it2 != it->end(); it2++) {
            if ((*it2)->shown()) {
                (*it2)->resize(xx, yy, minature_w, minature_h);
            }
            xx += minature_w + 5;
        }
        xx = sx;
        yy += minature_h + 5;
    }
}

FramePanel *ResultsPanel::getFrame(int col, int row) {
    if (row >= miniatures_.size()) 
        return NULL;
    if (col >= miniatures_.begin()->size()) 
        return NULL;
    auto it = miniatures_.begin();
    std::advance(it, row);
    auto it2 = it->begin();
    std::advance(it2, col);
    return *(it2);
}


} // namespace dexpert
