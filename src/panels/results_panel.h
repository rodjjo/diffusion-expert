/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_RESULTS_PANEL_H_
#define SRC_PANELS_RESULTS_PANEL_H_

#include <string>

#include <list>
#include <FL/Fl_Group.H>
#include "src/panels/frame_panel.h"

namespace dexpert
{

class ResultsPanel: public Fl_Group {
 public:
    ResultsPanel(int x, int y, int w, int h);
    virtual ~ResultsPanel();
    void updatePanels();
    void resize(int x, int y, int w, int h) override;

 private:
    void alignComponents();
    FramePanel *getFrame(int col, int row);

 private:
    std::list<std::list<FramePanel *> > miniatures_;
};

}  // namespace dexpert

#endif  // SRC_PANELS_RESULTS_PANEL_H_