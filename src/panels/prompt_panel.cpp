#include <algorithm>

#include "src/stable_diffusion/state.h"
#include "src/dialogs/common_dialogs.h"
#include "src/config/config.h"
#include "src/data/xpm.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"

#include "src/panels/painting_panel.h"
#include "src/panels/prompt_panel.h"


namespace dexpert
{

namespace {
    std::string last_prompt;
    std::string last_negative_prompt;
}

PromptPanel::PromptPanel(int x, int y, int w, int h, callback_t on_generate) : EventListener(), Fl_Group(x, y, w, h), on_generate_(on_generate) {
    this->begin();
    positivePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Prompt");
    negativePrompt_ = new Fl_Multiline_Input( 0, 0, 1, 1, "Negative Prompt");
    generateBtn_.reset(new Button(xpm::image(xpm::button_play), [this] {
        this->on_generate_();
    }));
    interrogateBtn1_.reset(new Button("?", [this] {
        this->interrogate("Clip");
    }));
    interrogateBtn2_.reset(new Button("??", [this] {
        this->interrogate("DeepBooru");
    }));
    seed_ = new Fl_Int_Input(0, 0, 1, 1, "Seed");
    steps_ = new Fl_Int_Input(0, 0, 1, 1, "Steps");
    guidance_ = new Fl_Float_Input(0, 0, 1, 1, "CFG");
    var_strength_ = new Fl_Float_Input(0, 0, 1, 1, "Var Strength");
    width_ = new Fl_Int_Input(0, 0, 1, 1, "Width");
    height_ = new Fl_Int_Input(0, 0, 1, 1, "Height");
    models_ = new Fl_Choice(0, 0, 1, 1, "Model");
    modelsInpaint_ = new Fl_Choice(0, 0, 1, 1, "Inpainting Model");
    restore_face_ = new Fl_Check_Button(0, 0, 1, 1, "Restore faces");

    textualPanel_ = new EmbeddingPanel(embedding_textual_inv, 0, 0, 1, 1);

    loraPanel_ = new EmbeddingPanel(embedding_lora, 0, 0, 1, 1);
    
    this->end();

    positivePrompt_->align(FL_ALIGN_TOP_LEFT);
    negativePrompt_->align(FL_ALIGN_TOP_LEFT);
    seed_->align(FL_ALIGN_TOP_LEFT);
    steps_->align(FL_ALIGN_TOP_LEFT);
    guidance_->align(FL_ALIGN_TOP_LEFT);
    width_->align(FL_ALIGN_TOP_LEFT);
    height_->align(FL_ALIGN_TOP_LEFT);
    models_->align(FL_ALIGN_TOP_LEFT);
    modelsInpaint_->align(FL_ALIGN_TOP_LEFT);
    var_strength_->align(FL_ALIGN_TOP_LEFT);

    positivePrompt_->value(last_prompt.c_str());
    negativePrompt_->value(last_negative_prompt.c_str());
    generateBtn_->tooltip("Generate a new image.");
    
    interrogateBtn1_->tooltip("Interrogate Clip");
    interrogateBtn2_->tooltip("Interrogate DeepBooru");

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
    last_prompt = positivePrompt_->value();
    return positivePrompt_->value();
}

const char *PromptPanel::getNegativePrompt() {
    last_negative_prompt = negativePrompt_->value();
    return negativePrompt_->value();
}

void PromptPanel::interrogate(const char* model) {
    if (!image_panel_) {
        return;
    }
    if (!image_panel_->getImage()) {
        show_error("No input image to interrogate!");
        return;
    }
    std::string result;
    const char* error = NULL;
    dexpert::py::get_py()->execute_callback(
        dexpert::py::interrogate_image(
            model, image_panel_->getImage(), [&error, &result] (bool success, const char *msg, const std::string& prompt) {
                if (!success) {
                    error = msg;
                } else {
                    result = prompt;
                }
            }
        )
    );
    if (error) {
        show_error(error);
    } else {
        positivePrompt_->value(result.c_str());
    }
}

void PromptPanel::setImagePanel(PaintingPanel *panel) {
    image_panel_ = panel;
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

    positivePrompt_->resize(x() + 5, y() + 25, w() - 67 - 55, 50);

    interrogateBtn1_->size(50, 24);
    interrogateBtn2_->size(50, 24);
    interrogateBtn1_->position(positivePrompt_->x() + positivePrompt_->w() + 5, positivePrompt_->y());
    interrogateBtn2_->position(interrogateBtn1_->x(), interrogateBtn1_->y() + interrogateBtn1_->h() + 2);

    generateBtn_->position(
        interrogateBtn1_->x() + interrogateBtn1_->w() + 5, 
        positivePrompt_->y()
    );
    generateBtn_->size(50, 50);
    
    negativePrompt_->resize(
        x() + 5, 
        positivePrompt_->y() + positivePrompt_->h() + 25, 
        positivePrompt_->w() + 55, 
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
        300,
        25
    );
    modelsInpaint_->resize(
        models_->x() + 5 + models_->w(),
        models_->y(),
        300,
        25
    );
    restore_face_->resize(
        x() + 5,
        models_->y() + models_->h() + 5,
        120,
        25
    );
    textualPanel_->resize(x() + 5, restore_face_->y() + restore_face_->h() + 3, w() - 10, 160);
    loraPanel_->resize(x() + 5, textualPanel_->y() + textualPanel_->h() + 3, w() - 10, 160);
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
    
    std::string modelName = "";
    std::string modelInpaint = "";
    if (models_->value() >= 0) {
        modelName = models_->text(models_->value());
    } else {
        modelName = getConfig().getLatestSdModel();
    }
    if (modelsInpaint_->value() >= 0) {
        modelInpaint = modelsInpaint_->text(modelsInpaint_->value());
    } else {
        modelInpaint = getConfig().getLatestSdModelInpaint();
    }

    models_->clear();
    modelsInpaint_->clear();
    
    auto mdls = get_sd_state()->getSdModels();

    int index = 0;
    int inpaintIndex = 0;
    int value = -1;
    int valueInpaint = -1;
    std::string name_lower;

    for (auto it = mdls.cbegin(); it != mdls.cend(); it++) {
        name_lower = it->name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        if (name_lower.find("inpaint") != std::string::npos) {
            modelsInpaint_->add(it->name.c_str());
            if (valueInpaint == -1 && it->name == modelInpaint) {
                valueInpaint = inpaintIndex;
            }
            ++inpaintIndex;
        } else {
            models_->add(it->name.c_str());
            if (value == -1 && it->name == modelName) {
                value = index;
            }
            ++index;
        }
    }
    
    if (value < 0) 
        value = 0;

    if (valueInpaint < 0)
        valueInpaint = 0;

    models_->value(value); 
    modelsInpaint_->value(valueInpaint);
}

const char *PromptPanel::getSdModel(bool for_inpainting) {
    bool hasInpaint = modelsInpaint_->value() >= 0;
    bool hasNormal = models_->value() >= 0;

    if (hasInpaint) {
        dexpert::getConfig().setLatestSdModelInpaint(modelsInpaint_->text(modelsInpaint_->value()));
    }
    if (hasNormal) {
        dexpert::getConfig().setLatestSdModel(models_->text(models_->value()));
    }   
    if (hasNormal || hasInpaint) {
        dexpert::getConfig().save();
    }

    if (for_inpainting) {
        if (hasInpaint) {
            return modelsInpaint_->text(modelsInpaint_->value());
        }
    } else {
        if (hasNormal) {
            return models_->text(models_->value());
        }
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

bool PromptPanel::shouldRestoreFaces() {
    return restore_face_->value() != 0;
}

void PromptPanel::setImageSize(int w, int h) {
    char buffer[30] = {0,};
    sprintf(buffer, "%d", w);
    width_->value(buffer);
    sprintf(buffer, "%d", h);
    height_->value(buffer);
}

void PromptPanel::event_trigged(const void *sender, int event, void *data) {
    if (sender == textualPanel_) {
        if (event == embedding_event_selected) {
            auto item = textualPanel_->getSelectedEmbedding();
            if (!item) {
                return;
            }
            std::string text = positivePrompt_->value();
            text += " " + item->name;
            positivePrompt_->value(text.c_str());
        } else if (event == embedding_define_image) {
            if (image_panel_) {
                auto img = image_panel_->getImage();
                if (img) {
                    textualPanel_->setSelectedImage(img->resizeInTheCenter(100, 100));
                } else {
                    show_error("No input image to set");
                }
            }
        } else if (event == embedding_event_reload) {
            should_reload_model_ = true;
        }
    } else if (sender == loraPanel_) {
        if (event == embedding_event_selected) {
            auto item = loraPanel_->getSelectedEmbedding();
            if (!item) {
                return;
            }
            std::string text = positivePrompt_->value();
            text += " <" + item->name + ":1.0>";
            positivePrompt_->value(text.c_str());
        } else if (event == embedding_define_image) {
            if (image_panel_) {
                auto img = image_panel_->getImage();
                if (img) {
                    loraPanel_->setSelectedImage(img->resizeInTheCenter(100, 100));
                } else {
                    show_error("No input image to set");
                }
            }
        } else if (event == embedding_event_reload) {
            should_reload_model_ = true;
        }
    }
}

bool PromptPanel::shouldReload(bool clear) {
    if (should_reload_model_) {
        if (clear) {
            should_reload_model_ = false;
        }
        return true;
    }
    return false;
}

} // namespace dexpert
