/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PANTING_PANEL_H_
#define SRC_PANELS_PANTING_PANEL_H_

#include <memory>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>

#include "src/controls/button.h"
#include "src/panels/frame_panel.h"
#include "src/panels/prompt_panel.h"

namespace dexpert {

typedef enum {
   paiting_disabled,
   panting_img2img,
   panting_inpaint_masked,
   panting_inpaint_not_masked,
   panting_scribble,
   panting_canny,

   // keep panting_mode_max ath the end
   panting_mode_max
} panting_mode_t;

class PantingPanel: public Fl_Group {
 public:
    PantingPanel(int x, int y, int w, int h, PromptPanel *prompt);
    virtual ~PantingPanel();
    void resize(int x, int y, int w, int h) override;

 private:
    void alignComponents();
    void saveImage();
    void saveMask();
    void openImage();
    void openMask();

 private:
    PromptPanel *prompt_;
    Fl_Group *left_bar_;
    Fl_Choice *mode_;
    Fl_Box* label_image_;
    Fl_Box* label_mask_;
    FramePanel *image_panel_;
    std::unique_ptr<Button> btnOpen_;
    std::unique_ptr<Button> btnSave_;
    std::unique_ptr<Button> btnOpenMask_;
    std::unique_ptr<Button> btnSaveMask_;
};
    
}   // namespace dexpert


#endif  // SRC_PANELS_PANTING_PANEL_H_