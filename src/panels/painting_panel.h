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
#include <FL/Fl_Float_Input.H>

#include "src/controls/button.h"
#include "src/controls/image_panel.h"
#include "src/panels/frame_panel.h"
#include "src/panels/prompt_panel.h"
#include "src/python/raw_image.h"
#include "src/stable_diffusion/generator.h"
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
   painting_deepth,

   // keep painting_mode_max ath the end
   painting_mode_max
} painting_mode_t;

typedef enum {
   brush_size_disabled,
   brush_size_1,
   brush_size_2,
   brush_size_4,
   brush_size_8,
   brush_size_16,
   brush_size_32,
   // 
   brush_size_count
} brush_size_t;

class PaintingPanel;

class PaintingPanel: public Fl_Group {
 public:
    PaintingPanel(int x, int y, int w, int h, PromptPanel *prompt, PaintingPanel *inputPanel = NULL, bool only_control_net=false);
    virtual ~PaintingPanel();
    void resize(int x, int y, int w, int h) override;
    void setImage(RawImage *image);
    RawImage *getImage();

    std::shared_ptr<ControlNet> getControlnet();
    RawImage* getImg2ImgImage();
    RawImage* getImg2ImgMask();
    RawImage* getImg2ImgControl();
    float get_denoise_strength();
    bool shouldInpaintMasked();
    bool ready();
    painting_mode_t getSelectedMode();
    bool maskBlurEnabled();
    inpaint_mode_t getInpaintMode();

 private:
   static void modeSelected(Fl_Widget *widget, void *cbdata);
   void modeSelected();
   static void brushSelected(Fl_Widget *widget, void *cbdata);
   void brushSelected();

 private:
    void alignComponents();
    void saveImage();
    void useInputImage();
    void saveMask();
    void openImage();
    void newMask();
    void openMask();
    void extractCanny();
    void extractScribble();
    void extractPose();
    void extractDeepth();
    bool ensureControlPresent();
    bool ensureImagePresent();
    bool ensureMaskPresent();
    void pre_process(const char* method);
    void enableControls();

 private:
    bool only_control_net_;
    PaintingPanel *inputPanel_;
    PromptPanel *prompt_;
    Fl_Group *left_bar_;
    Fl_Choice *mode_;
    Fl_Choice *brushes_;
    Fl_Float_Input *denoise_;
    Fl_Box* label_image_;
    Fl_Box* label_mask_;
    Fl_Box* label_control_;
    Fl_Check_Button *draw_image_check_;
    Fl_Check_Button *blur_mask_;
    Fl_Choice *inpaintMode_;
    ImagePanel *image_panel_;
    std::unique_ptr<Button> btnOpen_;
    std::unique_ptr<Button> btnSave_;
    std::unique_ptr<Button> btnInput_;
    std::unique_ptr<Button> btnNewMask_;
    std::unique_ptr<Button> btnOpenMask_;
    std::unique_ptr<Button> btnSaveMask_;
    std::unique_ptr<Button> btnScribble_;
    std::unique_ptr<Button> btnCanny_;
    std::unique_ptr<Button> btnPose_;
    std::unique_ptr<Button> btnDeepth_;
};
    
}   // namespace dexpert


#endif  // SRC_PANELS_PANTING_PANEL_H_