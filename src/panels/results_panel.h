/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_RESULTS_PANEL_H_
#define SRC_PANELS_RESULTS_PANEL_H_

#include <string>

#include <vector>
#include <FL/Fl_Group.H>
#include "src/panels/preview_panel.h"
#include "src/data/event_manager.h"
#include "src/panels/painting_panel.h"


namespace dexpert
{

class ResultsPanel: public EventListener, public Fl_Group {
 public:
    ResultsPanel(int x, int y, int w, int h, PaintingPanel *painting);
    virtual ~ResultsPanel();
    void updatePanels();
    void resize(int x, int y, int w, int h) override;
 protected:
  void event_trigged(const void *sender, int event, void *data) override;
 private:
   void alignComponents();
   PreviewPanel *getFrame(int col, int row);
   
 private:
  PaintingPanel *painting_;
  std::vector<std::vector<PreviewPanel *> > miniatures_;
};

}  // namespace dexpert

#endif  // SRC_PANELS_RESULTS_PANEL_H_
