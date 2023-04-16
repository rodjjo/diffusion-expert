#include "src/panels/pages.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"
#include "src/stable_diffusion/generator_txt2img.h"

namespace dexpert {
namespace  {
    const char *kPAGE_TITLES[page_max] = {
        "Prompt",
        "Input image",
        "Controlnet 1",
        "Controlnet 2",
        "Controlnet 3",
        "Controlnet 4",
        "Results"
    };
} 

Pages::Pages(int x, int y, int w, int h) : Fl_Group(x, y, w, h, "") {
    for (int i = 0; i < page_max; ++i)  {
        visible_pages_[i] = false;
        pages_[i] =  NULL; 
    }

    this->begin();

    resultsPanel_ = new ResultsPanel(0, 0, 1, 1);
    pages_[page_results] = resultsPanel_;
    promptPanel_ = new PromptPanel(0, 0, 1, 1);
    pages_[page_prompts] = promptPanel_;
    inputImage_ = new PantingPanel(0, 0, 1, 1, promptPanel_);
    pages_[page_input_image] = inputImage_;
    
    visible_pages_[page_results] = true;
    visible_pages_[page_prompts] = true;
    visible_pages_[page_input_image] = true;

    for (int i = 0; i < page_max; ++i) {
        if (pages_[i]) {
            pages_[i]->hide();
            continue;
        }
        pages_[i] = new Fl_Group(0, 0, 1, 1, "");
        pages_[i]->hide();
    }

    this->end();

    alignComponents();
    goPage(page_results);
}

Pages::~Pages() {
}

const char *Pages::pageTitle(page_t page) {
    return kPAGE_TITLES[page];
}

void Pages::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
};

void Pages::alignComponents() {
    for (int i = 0; i < page_max; ++i) {
        pages_[i]->resize(x(), y(), w(), h());
    }
}

bool Pages::isVisible(page_t page) {
    return visible_pages_[page];
}

int Pages::pageCount() {
    int result = 0;
    for (int i = 0; i < page_max; ++i) {
        if (isVisible((page_t) i)) {
            ++result;
        }
    }
    return result;
}

bool Pages::goPage(page_t page) {
    bool result = false;
    if (isVisible(page)) {
        result = true;
        active_page_ = page;
    } 
    for (int i = 0; i < page_max; ++i) {
        if (i == active_page_) {
            pages_[i]->show();
        } else {
            pages_[i]->hide();
        }
    }
    if (active_page_ == page_results) {
        resultsPanel_->updatePanels();
    }
    return result;
}

page_t Pages::getPageIndex(int index) {
    int page_number = 0;
    for (int i = 0; i < page_max; ++i) {
         if (isVisible((page_t) i)) {
            if (index == page_number) {
                return (page_t) i;
            }
            ++page_number;
        }
    }
    return page_prompts;
}

page_t Pages::activePage() {
    return active_page_;
}

int Pages::visibleIndex() {
    int page_number = 0;
    for (int i = 0; i < page_max; ++i) {
         if (isVisible((page_t) i)) {
            if (active_page_ == i) {
                return page_number;
            }
            ++page_number;
        }
    }
    return page_prompts;
}

void Pages::textToImage() {
    int seed = promptPanel_->getSeed();
    if (seed == -1) 
        seed = get_sd_state()->randomSeed();

    const char* model = promptPanel_->getSdModel();
    if (model == NULL) {
        show_error("Add a model file (.safetensors or .ckpt) into 'models/stable diffusion' directory before you start!");
        return;
    }

    get_sd_state()->setSdModel(model);

    auto g = std::make_shared<GeneratorTxt2Image>(
        promptPanel_->getPrompt(),
        promptPanel_->getNegativePrompt(),
        get_sd_state()->getSdModelPath(model),
        seed,
        promptPanel_->getWidth(),
        promptPanel_->getHeight(),
        promptPanel_->getSteps(),
        promptPanel_->getCFG(),
        promptPanel_->getVariationStrength()
    );

    if (!get_sd_state()->generatorAdd(g)) {
        show_error(get_sd_state()->lastError());
    } else if (active_page_ == page_results) {
        resultsPanel_->updatePanels();
    }
}

}   // namespace dexpert
