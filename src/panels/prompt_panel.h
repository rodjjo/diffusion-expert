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

#include "src/data/event_manager.h"
#include "src/controls/button.h"
#include "src/panels/embedding_panel.h"

namespace dexpert
{

class PaintingPanel;

typedef std::function<void()>  callback_t;

class PromptPanel: public EventListener, public Fl_Group {
 public:
    PromptPanel(int x, int y, int w, int h);
    virtual ~PromptPanel();
    const char *getPrompt();
    const char *getNegativePrompt();
    const char *getSdModel(bool for_inpainting);
    int getSeed();
    int getSteps();
    int getWidth();
    int getHeight();
    float getCFG();
    float getVariationStrength();
    bool shouldRestoreFaces();
    bool ready(bool require_prompt);
    void setImageSize(int w, int h);
    void refreshModels();
    void setImagePanel(PaintingPanel *panel);
    bool shouldReload(bool clear);
 private:
    void alignComponents();
    void interrogate(const char* model);
    
 protected:
    void resize(int x, int y, int w, int h) override;
    void event_trigged(const void *sender, int event, void *data) override;

 private:
   bool should_reload_model_;
   PaintingPanel* image_panel_ = NULL;
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
   Fl_Choice *modelsInpaint_;
   Fl_Check_Button *restore_face_;
   EmbeddingPanel *textualPanel_;
   EmbeddingPanel *loraPanel_;
   std::unique_ptr<Button> interrogateBtn1_;
   std::unique_ptr<Button> interrogateBtn2_;
};


 } // namespace dexpert

#endif  // SRC_PANELS_PROMPT_PANEL_H_
