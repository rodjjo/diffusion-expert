#pragma once

#include <string>

#include <FL/FL_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>

namespace dfe
{

class PromptFrame {
public:
    PromptFrame(Fl_Group *parent);
    ~PromptFrame();

    void alignComponents();
    std::string positive_prompt();
    std::string negative_prompt();
    std::string get_model();
    std::string get_inpaint_model();
    int get_seed();
    int get_batch_size();
    int get_steps();
    float get_cfg();
    int get_width();
    int get_height();
    bool use_lcm_lora();
    bool use_tiny_vae();

    bool validate();
    void refresh_models();
private:
    Fl_Group             *parent_;
    Fl_Multiline_Input   *positive_input_;
    Fl_Multiline_Input   *negative_input_;
    Fl_Int_Input         *seed_input_;
    Fl_Int_Input         *batch_input_;
    Fl_Int_Input         *steps_input_;
    Fl_Float_Input       *guidance_input_;
    Fl_Int_Input         *width_input_;
    Fl_Int_Input         *height_input_;
    Fl_Choice            *models_input_;
    Fl_Choice            *modelsInpaint_input_;
    Fl_Check_Button      *use_lcm_lora_;
    Fl_Check_Button      *use_tiny_vae_;
};

} // namespace dfe
