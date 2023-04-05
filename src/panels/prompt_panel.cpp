#include "src/panels/prompt_panel.h"

namespace dexpert
{

PromptPanel::PromptPanel(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    this->begin();
    positivePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Prompt:");
    negativePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Negative Prompt:");
    this->end();
    positivePrompt_->align(FL_ALIGN_TOP_LEFT);
    negativePrompt_->align(FL_ALIGN_TOP_LEFT);
    positivePrompt_->value("An astrounaut riding a horse at the moon");
    negativePrompt_->value("drawing,cartoon,3d,render,rendering");
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

void PromptPanel::alignComponents() {
    positivePrompt_->resize(x() + 5, y() + 25, w() - 10, 50);
    negativePrompt_->resize(
        x() + 5, 
        positivePrompt_->y() + positivePrompt_->h() + 25, 
        w() - 10, 
        positivePrompt_->h()
    );
}

void PromptPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

int PromptPanel::minimalHeight() {
    return negativePrompt_->y() + negativePrompt_->h() + 10;
}
} // namespace dexpert
