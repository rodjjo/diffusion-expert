/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PANTING_PANEL_H_
#define SRC_PANELS_PANTING_PANEL_H_

#include <memory>

#include <FL/Fl_Group.H>

#include "src/controls/button.h"
#include "src/panels/frame_panel.h"

namespace dexpert {

class PantingPanel: public Fl_Group {
 public:
    PantingPanel(int x, int y, int w, int h);
    virtual ~PantingPanel();
    void resize(int x, int y, int w, int h) override;

 private:
    void alignComponents();

 private:
    Fl_Group *left_bar_;
    FramePanel *image_panel_;
    std::unique_ptr<Button> btnOpen_;
    std::unique_ptr<Button> btnSave_;
};
    
}   // namespace dexpert


#endif  // SRC_PANELS_PANTING_PANEL_H_