/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PROMPT_PANEL_H_
#define SRC_PANELS_PROMPT_PANEL_H_

#include <functional>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>

#include "src/controls/button.h"

namespace dexpert
{

typedef std::function<void()>  callback_t;

class PromptPanel: public Fl_Group {
 public:
    PromptPanel(int x, int y, int w, int h, callback_t on_generate);
    virtual ~PromptPanel();
    const char *getPrompt();
    const char *getNegativePrompt();
    const char *getSdModel();
    int getSeed();
    int getSteps();
    int getWidth();
    int getHeight();
    float getCFG();
    float getVariationStrength();
    bool shouldRestoreFaces();
    bool shouldUseCodeformer();
    bool ready(bool require_prompt);
    void setImageSize(int w, int h);

 private:
    void alignComponents();
    void refreshModels();
    
 protected:
    void resize(int x, int y, int w, int h) override;

 private:
   callback_t on_generate_;
   Fl_Multiline_Input *positivePrompt_;
   Fl_Multiline_Input *negativePrompt_;
   Fl_Int_Input *seed_;
   Fl_Int_Input *steps_;
   Fl_Float_Input *guidance_;
   Fl_Float_Input *var_strength_;
   Fl_Int_Input *width_;
   Fl_Int_Input *height_;
   Fl_Choice *models_;
   Fl_Check_Button *restore_face_;
   Fl_Check_Button *codeformer_;
   std::unique_ptr<Button> generateBtn_;
};


 } // namespace dexpert

#endif  // SRC_PANELS_PROMPT_PANEL_H_
