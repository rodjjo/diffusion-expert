#include "python/routines.h"

#include "frames/prompt_frame.h"


namespace dfe
{

namespace {
    const char *resize_mode_texts[resize_mode_count] = {
        "Do not scale down the image",
        "Scale down to fit in 512x512",
        "Scale down to fit in 768x768",
        "Scale down to fit in 1024x1024",
        "Scale down to fit in 1280x1280"
    };
}
    

PromptFrame::PromptFrame(Fl_Group *parent) {
    parent_ = parent;

    positive_input_ = new Fl_Multiline_Input(0, 0, 1, 1, "Positive prompt");
    negative_input_ = new Fl_Multiline_Input(0, 0, 1, 1, "Negative Prompt");
    seed_input_ = new Fl_Int_Input(0, 0, 1, 1, "Seed");
    batch_input_ = new Fl_Int_Input(0, 0, 1, 1, "Batch size");
    steps_input_ = new Fl_Int_Input(0, 0, 1, 1, "Steps");
    guidance_input_ = new Fl_Float_Input(0, 0, 1, 1, "CFG");
    width_input_ = new Fl_Int_Input(0, 0, 1, 1, "Width");
    height_input_ = new Fl_Int_Input(0, 0, 1, 1, "Height");
    models_input_ = new Fl_Choice(0, 0, 1, 1, "Model");
    schedulers_ =  new Fl_Choice(0, 0, 1, 1, "Scheduler");
    resizeModes_ =  new Fl_Choice(0, 0, 1, 1, "Resize mode");
    modelsInpaint_input_ = new Fl_Choice(0, 0, 1, 1, "Inpaint Model");
    use_lcm_lora_ = new Fl_Check_Button(0, 0, 1, 1, "Use LCM lora");
    use_tiny_vae_ = new Fl_Check_Button(0, 0, 1, 1, "Use Tiny AutoEncoder");

    positive_input_->align(FL_ALIGN_TOP_LEFT);
    negative_input_->align(FL_ALIGN_TOP_LEFT);
    seed_input_->align(FL_ALIGN_TOP_LEFT);
    batch_input_->align(FL_ALIGN_TOP_LEFT);
    steps_input_->align(FL_ALIGN_TOP_LEFT);
    guidance_input_->align(FL_ALIGN_TOP_LEFT);
    width_input_->align(FL_ALIGN_TOP_LEFT);
    height_input_->align(FL_ALIGN_TOP_LEFT);
    models_input_->align(FL_ALIGN_TOP_LEFT);
    schedulers_->align(FL_ALIGN_TOP_LEFT);
    resizeModes_->align(FL_ALIGN_TOP_LEFT);
    modelsInpaint_input_->align(FL_ALIGN_TOP_LEFT);

    seed_input_->value("-1");
    batch_input_->value("1");
    steps_input_->value("25");
    guidance_input_->value("7.5");
    width_input_->value("512");
    height_input_->value("512");

    use_lcm_lora_->callback(widget_cb, this);

    for (int i = 0; i < resize_mode_count; i++) {
        resizeModes_->add(resize_mode_texts[i]);
    }
    resizeModes_->value(resize_fit_1024x1024);
    resizeModes_->tooltip("Scale down the image before processing it");
    
    alignComponents();
}

PromptFrame::~PromptFrame() {

}

void PromptFrame::widget_cb(Fl_Widget* widget, void *cbdata) {
    static_cast<PromptFrame *>(cbdata)->widget_cb(widget);
}

void PromptFrame::widget_cb(Fl_Widget* widget) {
    if (widget == use_lcm_lora_) {
        if (use_lcm_lora()) {
            if (get_steps() > 8) {
                steps_input_->value("4");
            }
            if (get_cfg() > 2.0) {
                guidance_input_->value("2.0");
            }
            if (get_scheduler() != "LCMScheduler") {
                auto idx = schedulers_->find_index("LCMScheduler");
                if (idx >= 0) {
                    schedulers_->value(idx);
                }
            }
        } else {
            if (get_steps() < 20) {
                steps_input_->value("25");
            }
            if (get_cfg() < 6.0) {
                guidance_input_->value("7.5");
            }
            if (get_scheduler() == "LCMScheduler") {
                auto idx = schedulers_->find_index("EulerAncestralDiscreteScheduler");
                if (idx >= 0) {
                    schedulers_->value(idx);
                }
            }

        }
    }
}

void PromptFrame::alignComponents() {
    int sx = parent_->x(), sy = parent_->y();
    int pw = parent_->w(), ph = parent_->h();
    if (pw > 860) {
        pw = 860;
        sx = sx + (parent_->w() - 860);
    }
    positive_input_->resize(sx + 5, sy + 35, pw - 10, 50);
    negative_input_->resize(sx + 5, positive_input_->y() + 75, positive_input_->w(), positive_input_->h());
    seed_input_->resize(sx + 5, negative_input_->y() + 75, (pw - 20) / 3, 20);
    batch_input_->resize(seed_input_->x() + seed_input_->w() + 5, seed_input_->y(), seed_input_->w(), seed_input_->h());
    steps_input_->resize(batch_input_->x() + batch_input_->w() + 5, batch_input_->y(), batch_input_->w(), batch_input_->h());
    guidance_input_->resize(sx + 5, steps_input_->y() + 45, steps_input_->w(), steps_input_->h());
    width_input_->resize(guidance_input_->x() + guidance_input_->w() + 5, guidance_input_->y(), guidance_input_->w(), guidance_input_->h());
    height_input_->resize(width_input_->x() + width_input_->w() + 5, width_input_->y(), width_input_->w(), width_input_->h());
    models_input_->resize(sx + 5, height_input_->y() + 45, (pw - 15) / 2, height_input_->h());
    modelsInpaint_input_->resize(models_input_->x() + models_input_->w() + 5, models_input_->y(), models_input_->w(), models_input_->h());
    use_lcm_lora_->resize(sx + 5, modelsInpaint_input_->y() + modelsInpaint_input_->h() + 5, 160, 20);
    use_tiny_vae_->resize(use_lcm_lora_->x() + use_lcm_lora_->w() + 5, use_lcm_lora_->y(), use_lcm_lora_->w(), use_lcm_lora_->h());
    schedulers_->resize(sx + 5, use_tiny_vae_->y() + use_tiny_vae_->h() + 20, models_input_->w(), models_input_->h());
    resizeModes_->resize( schedulers_->x() +  schedulers_->w() + 5, schedulers_->y(), models_input_->w(), models_input_->h());
}


std::string PromptFrame::positive_prompt() {
    return positive_input_->value();
}

std::string PromptFrame::negative_prompt() {
    return negative_input_->value();
}

std::string PromptFrame::get_model() {
    if (models_input_->value() >= 0) {
        return models_input_->text(models_input_->value());
    }
    return std::string();
}

std::string PromptFrame::get_inpaint_model() {
    if (modelsInpaint_input_->value() >= 0) {
        return modelsInpaint_input_->text(modelsInpaint_input_->value());
    }
    return std::string();
}

std::string PromptFrame::get_scheduler() {
    if (schedulers_->value() >= 0) {
        return schedulers_->text(schedulers_->value());
    }
    return std::string();
}

int PromptFrame::get_seed() {
    int result = -1;
    sscanf(seed_input_->value(), "%d", &result);
    return result;
}

int PromptFrame::get_batch_size() {
    int result = 1;
    sscanf(batch_input_->value(), "%d", &result);
    
    if (result < 1) {
        result = 1;
    } else if (result > 8) {
        result = 8;
    } else {
        return result;
    }
    char buffer[25] = "";
    sprintf(buffer, "%d", result);
    batch_input_->value(buffer);
    return result;
}

int PromptFrame::get_steps() {
    int result = 30;
    sscanf(steps_input_->value(), "%d", &result);
    return result;
}

float PromptFrame::get_cfg() {
    float result = 7.5;
    sscanf(guidance_input_->value(), "%f", &result);
    return result;
}

int PromptFrame::get_width() {
    int result = 512;
    sscanf(width_input_->value(), "%d", &result);
    return result;
}

int PromptFrame::get_height() {
    int result = 512;
    sscanf(height_input_->value(), "%d", &result);
    return result;
}

bool PromptFrame::validate() {
    return true;
}

bool PromptFrame::use_lcm_lora() {
    return use_lcm_lora_->value() != 0;
}

bool PromptFrame::use_tiny_vae() {
    return use_tiny_vae_->value() != 0;
}

void PromptFrame::refresh_models() {
    auto model_list = py::list_models();
    models_input_->clear();
    modelsInpaint_input_->clear();
    for (auto & m : model_list) {
        if (m.first) {
            modelsInpaint_input_->add(m.second.c_str());
        } else {
            models_input_->add(m.second.c_str());
        }
    }
    models_input_->value(0);
    modelsInpaint_input_->value(0);

    auto scheduler_list = py::list_schedulers();
    schedulers_->clear();
    for (auto & s : scheduler_list) {
        schedulers_->add(s.c_str());
    }
    schedulers_->value(0);
}

} // namespace dfe
