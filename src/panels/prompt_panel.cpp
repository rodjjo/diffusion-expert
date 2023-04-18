#include "src/panels/prompt_panel.h"
#include "src/stable_diffusion/state.h"
#include "src/dialogs/common_dialogs.h"
#include "src/config/config.h"

namespace dexpert
{

PromptPanel::PromptPanel(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    this->begin();
    positivePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Prompt");
    negativePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Negative Prompt");
    seed_ = new Fl_Int_Input(0, 0, 1, 1, "Seed");
    steps_ = new Fl_Int_Input(0, 0, 1, 1, "Steps");
    guidance_ = new Fl_Float_Input(0, 0, 1, 1, "CFG");
    var_strength_ = new Fl_Float_Input(0, 0, 1, 1, "Var Strength");
    width_ = new Fl_Int_Input(0, 0, 1, 1, "Width");
    height_ = new Fl_Int_Input(0, 0, 1, 1, "Height");
    models_ = new Fl_Choice(0, 0, 1, 1, "Model");
    this->end();

    positivePrompt_->align(FL_ALIGN_TOP_LEFT);
    negativePrompt_->align(FL_ALIGN_TOP_LEFT);
    seed_->align(FL_ALIGN_TOP_LEFT);
    steps_->align(FL_ALIGN_TOP_LEFT);
    guidance_->align(FL_ALIGN_TOP_LEFT);
    width_->align(FL_ALIGN_TOP_LEFT);
    height_->align(FL_ALIGN_TOP_LEFT);
    models_->align(FL_ALIGN_TOP_LEFT);
    var_strength_->align(FL_ALIGN_TOP_LEFT);

    positivePrompt_->value("An astronaut riding a horse at the moon");
    negativePrompt_->value("drawing,cartoon,3d,render,rendering");

    seed_->value("-1");
    steps_->value("50");
    guidance_->value("7.5");
    var_strength_->value("0.1");
    width_->value("512");
    height_->value("512");

    alignComponents();
    refreshModels();
}

PromptPanel::~PromptPanel() {

}

const char *PromptPanel::getPrompt() {
    return positivePrompt_->value();
}

const char *PromptPanel::getNegativePrompt() {
    return negativePrompt_->value();
}

int PromptPanel::getSeed() {
    int result = -1;
    sscanf(seed_->value(), "%d", &result);
    return result;
}

int PromptPanel::getSteps() {
    int result = 50;
    sscanf(steps_->value(), "%d", &result);
    if (result < 1) {
        steps_->value("1");
        return 1;
    }
    if (result > 150) {
        steps_->value("150");
        return 150;
    }
    return result;
}

int PromptPanel::getWidth() {
    int result = 512;
    sscanf(width_->value(), "%d", &result);
    if (result < 2) {
        width_->value("2");
        return 2;
    }
    if (result > 2048) {
        width_->value("2048");
        return 2048;
    }
    if (result % 8) {
        result += 8 - result % 8;
        char buffer[30] = {0,};
        sprintf(buffer, "%d", result);
        width_->value(buffer);
    }
    return result;
}

int PromptPanel::getHeight() {
    int result = 512;
    sscanf(height_->value(), "%d", &result);
    if (result < 2) {
        height_->value("2");
        return 2;
    }
    if (result > 2048) {
        height_->value("2048");
        return 2048;
    }
    if (result % 8) {
        result += 8 - result % 8;
        char buffer[30] = {0,};
        sprintf(buffer, "%d", result);
        height_->value(buffer);
    }
    return result;
}

float PromptPanel::getCFG() {
    float result = 7.5;
    sscanf(guidance_->value(), "%f", &result);
    if (result < 0) {
        guidance_->value("0");
        return 0;
    }
    if (result > 100) {
        guidance_->value("100");
        return 100;
    }
    return result;
}

float PromptPanel::getVariationStrength() {
    float result = 0.1;
    sscanf(var_strength_->value(), "%f", &result);
    if (result < 0) {
        var_strength_->value("0");
        return 0;
    }
    if (result > 1) {
        var_strength_->value("1");
        return 1;
    }
    return result;
}

void PromptPanel::alignComponents() {
    positivePrompt_->resize(x() + 5, y() + 25, w() - 10, 50);
    negativePrompt_->resize(
        x() + 5, 
        positivePrompt_->y() + positivePrompt_->h() + 25, 
        w() - 10, 
        positivePrompt_->h()
    );
    seed_->resize(
        x() + 5, 
        negativePrompt_->y() + negativePrompt_->h() + 25, 
        75, 
        25
    );
    steps_->resize(
        seed_->x() + seed_->w() + 5, 
        seed_->y(),
        75,
        25
    );
    guidance_->resize(
        steps_->x() + steps_->w() + 5, 
        steps_->y(),
        75,
        25
    );
    width_->resize(
        guidance_->x() + guidance_->w() + 5, 
        guidance_->y(),
        75,
        25
    );
    height_->resize(
        width_->x() + width_->w() + 5, 
        width_->y(),
        75,
        25
    );
    var_strength_->resize(
        height_->x() + height_->w() + 5, 
        height_->y(),
        75,
        25
    );
    models_->resize(
        x() + 5,
        seed_->y() + seed_->h() + 25,
        420,
        25
    );
}

void PromptPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

void PromptPanel::refreshModels() {
    if (!get_sd_state()->reloadSdModelList()) {
        dexpert::show_error(get_sd_state()->lastError());
        return;
    }
    
    const char *modelName = "";
    if (models_->value() >= 0) {
        modelName = models_->text(models_->value());
    } else {
        modelName = getConfig().getLatestSdModel();
    }

    models_->clear();
    auto mdls = get_sd_state()->getSdModels();
    int index = 0;
    int value = -1;
    for (auto it = mdls.cbegin(); it != mdls.cend(); it++) {
        models_->add(it->name.c_str());
        if (value == -1 && it->name == modelName) {
            value = index;
        }
        ++index;
    }
    
    if (value < 0) 
        value = 0;

    models_->value(value); 
}

const char *PromptPanel::getSdModel() {
    if (models_->value() >= 0) {
        return models_->text(models_->value());
    }
    return NULL;
}

bool PromptPanel::ready(bool require_prompt) {
    if (require_prompt) {
        std::string p = getPrompt();
        if (p.empty()) {
            show_error("The prompt is required for text to image");
            return false;
        }
    }
    return true;
}

} // namespace dexpert
