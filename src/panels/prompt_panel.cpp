#include "src/panels/prompt_panel.h"

namespace dexpert
{

PromptPanel::PromptPanel(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    this->begin();
    positivePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Prompt:");
    negativePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Negative Prompt:");
    seed_ = new Fl_Int_Input(0, 0, 1, 1, "Seed:");
    this->end();
    positivePrompt_->align(FL_ALIGN_TOP_LEFT);
    negativePrompt_->align(FL_ALIGN_TOP_LEFT);
    seed_->align(FL_ALIGN_TOP_LEFT);
    positivePrompt_->value("An astronaut riding a horse at the moon");
    negativePrompt_->value("drawing,cartoon,3d,render,rendering");
    seed_->value("-1");
    alignComponents();
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
        80, 
        25
    );
}

void PromptPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

int PromptPanel::minimalHeight() {
    return seed_->y() + seed_->h() + 10;
}
} // namespace dexpert
