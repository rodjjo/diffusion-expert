#pragma once

#include <memory>
#include <string>

#include <FL/FL_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>

#include "frames/embeddings_frame.h"

namespace dfe
{

typedef enum {
    resize_not_resize,
    resize_fit_512x512,
    resize_fit_768x768,
    resize_fit_1024x1024,
    resize_fit_1280x1280,
    //  keep resize_mode_count at the end
    resize_mode_count
} resize_modes_t;

class PromptFrame: public SubscriberThis {
public:
    PromptFrame(Fl_Group *parent);
    ~PromptFrame();

    void alignComponents();
    std::string positive_prompt();
    std::string negative_prompt();
    std::string get_model();
    std::string get_inpaint_model();
    std::string get_scheduler();
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
    void insert_current_textual();
    void insert_current_lora();
    
protected:
    static void widget_cb(Fl_Widget* widget, void *cbdata);
    void widget_cb(Fl_Widget* widget);
    void dfe_handle_event(void *sender, event_id_t event, void *data) override;

private:
    std::unique_ptr<EmbeddingFrame> loras_;
    std::unique_ptr<EmbeddingFrame> embeddings_;
    Fl_Group             *parent_;
    Fl_Group             *lora_gp_;
    Fl_Group             *emb_gp_;
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
    Fl_Choice            *schedulers_;
    Fl_Choice            *resizeModes_;
    Fl_Check_Button      *use_lcm_lora_;
    Fl_Check_Button      *use_tiny_vae_;
};

} // namespace dfe
