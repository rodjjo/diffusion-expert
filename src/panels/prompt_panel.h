/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PROMPT_PANEL_H_
#define SRC_PANELS_PROMPT_PANEL_H_

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Int_Input.H>

namespace dexpert
{

class PromptPanel: public Fl_Group {
 public:
    PromptPanel(int x, int y, int w, int h);
    virtual ~PromptPanel();
    const char *getPrompt();
    const char *getNegativePrompt();
    int minimalHeight();
    int getSeed();

 private:
    void alignComponents();
    
 protected:
    void resize(int x, int y, int w, int h) override;

 private:
   Fl_Multiline_Input *positivePrompt_;
   Fl_Multiline_Input *negativePrompt_;
   Fl_Int_Input *seed_;
};


 } // namespace dexpert

#endif  // SRC_PANELS_PROMPT_PANEL_H_
