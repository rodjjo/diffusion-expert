/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PANTING_PANEL_H_
#define SRC_PANELS_PANTING_PANEL_H_

#include <memory>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>

#include "src/controls/button.h"
#include "src/panels/frame_panel.h"
#include "src/panels/prompt_panel.h"
#include "src/python/raw_image.h"
#include "src/stable_diffusion/controlnet.h"

namespace dexpert {

typedef enum {
   paiting_disabled,
   painting_img2img,
   painting_inpaint_masked,
   painting_inpaint_not_masked,
   painting_scribble,
   painting_canny,
   painting_pose,

   // keep painting_mode_max ath the end
   painting_mode_max
} painting_mode_t;

typedef enum {
   brush_size_disabled = 0,
   brush_size_1 = 1,
   brush_size_4 = 4,
   brush_size_8 = 8,
   brush_size_16 = 16,
   brush_size_32 = 32,
   brush_size_64 = 64,
   brush_size_128 = 128,
   // 
   brush_size_count = 8
} brush_size_t;


class PaintingPanel: public Fl_Group {
 public:
    PaintingPanel(int x, int y, int w, int h, PromptPanel *prompt);
    virtual ~PaintingPanel();
    void resize(int x, int y, int w, int h) override;
    void setImage(RawImage *image);
    
    std::shared_ptr<ControlNet> getControlnet();
    image_ptr_t getImg2ImgImage();
    image_ptr_t getImg2ImgMask();
    image_ptr_t getImg2ImgControl();
    bool should_invert_mask_colors();
    bool ready();


 private:
   static void modeSelected(Fl_Widget *widget, void *cbdata);
   void modeSelected();

 private:
    void alignComponents();
    void saveImage();
    void saveMask();
    void openImage();
    void newImage();
    void newMask();
    void openMask();
    void extractCanny();
    void extractScribble();
    void extractPose();
    bool ensureControlPresent();
    bool ensureImagePresent();
    bool ensureMaskPresent();
    void pre_process(const char* method);
    void enableControls();

 private:
    PromptPanel *prompt_;
    Fl_Group *left_bar_;
    Fl_Choice *mode_;
    Fl_Choice *brushes_;
    Fl_Box* label_image_;
    Fl_Box* label_mask_;
    Fl_Box* label_control_;
    Fl_Check_Button *draw_image_check_;
    FramePanel *image_panel_;
    std::unique_ptr<Button> btnNew_;
    std::unique_ptr<Button> btnOpen_;
    std::unique_ptr<Button> btnSave_;
    std::unique_ptr<Button> btnNewMask_;
    std::unique_ptr<Button> btnOpenMask_;
    std::unique_ptr<Button> btnSaveMask_;
    std::unique_ptr<Button> btnScribble_;
    std::unique_ptr<Button> btnCanny_;
    std::unique_ptr<Button> btnPose_;
};
    
}   // namespace dexpert


#endif  // SRC_PANELS_PANTING_PANEL_H_