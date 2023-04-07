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
    this->begin();
    for (int i = 0; i < page_max; ++i) {
        visible_pages_[i] = true;
        pages_[i] = new Fl_Group(0, 0, 1, 1, "");
        pages_[i]->end();
        pages_[i]->hide();
    }
    this->end();
    createInputImagePage();
    createPromptPage();
    createResultsPage();
    alignComponents();
    goPage(page_input_image);
}

Pages::~Pages() {
}

void Pages::createPromptPage() {
    auto pg = pages_[page_prompts];
    pg->begin();
    promptPanel_ = new PromptPanel(0, 0, 1, 1);
    pg->end();
}

void Pages::createInputImagePage() {
    auto pg = pages_[page_input_image];
    pg->begin();
    inputImage_ = new FramePanel(0, 0, 1, 1);
    pg->end();
}

void Pages::createResultsPage() {
    auto pg = pages_[page_results];
    pg->begin();
    resultsPanel_ = new ResultsPanel(0, 0, 1, 1);
    pg->end();
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
    auto pg = pages_[page_prompts];
    promptPanel_->position(pg->x(), pg->y());
    promptPanel_->size(pg->w(), promptPanel_->minimalHeight());
    pg = pages_[page_input_image];
    inputImage_->resize(pg->x(), pg->y(), pg->w(), pg->h());
    pg = pages_[page_results];
    resultsPanel_->resize(pg->x(), pg->y(), pg->w(), pg->h());
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
    return result;
}

page_t Pages::activePage() {
    return active_page_;
}


void Pages::openInputImage() {
    /*
    std::string path = choose_image_to_open(&current_open_input_dir_);
    if (!path.empty()) {
        if (!get_sd_state()->openInputImage(path.c_str())) {
            show_error(get_sd_state()->lastError());
        } else {
            inputImage_->redraw();
        }
    } 
    */
}

void Pages::saveInputImage() {
    /*
    if (!get_sd_state()->getInputImage()) {
        return;
    }
    std::string path = choose_image_to_save(&current_open_input_dir_);
    if (!path.empty()) {
        if (!get_sd_state()->saveInputImage(path.c_str())) {
            show_error(get_sd_state()->lastError());
        }
    }
    */
}

void Pages::textToImage() {
    int seed = promptPanel_->getSeed();
    if (seed == -1) 
        seed = get_sd_state()->randomSeed();
    get_sd_state()->setSdModel(promptPanel_->getSdModel());

    auto g = std::make_shared<GeneratorTxt2Image>(
        promptPanel_->getPrompt(),
        promptPanel_->getNegativePrompt(),
        get_sd_state()->getSdModelPath(promptPanel_->getSdModel()),
        seed,
        promptPanel_->getWidth(),
        promptPanel_->getHeight(),
        promptPanel_->getSteps(),
        promptPanel_->getCFG()
    );

    if (!get_sd_state()->generatorAdd(g)) {
        show_error(get_sd_state()->lastError());
    } else {
        resultsPanel_->updatePanels();
    }
}

}   // namespace dexpert
