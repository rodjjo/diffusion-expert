#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"
#include "src/stable_diffusion/generator_txt2img.h"
#include "src/stable_diffusion/generator_img2img.h"
#include "src/config/config.h"

#include "src/panels/pages.h"

#define COUNTOF(X) (sizeof((X))/(sizeof((X)[0])))

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

    promptPanel_ = new PromptPanel(0, 0, 1, 1);
    pages_[page_prompts] = promptPanel_;
    inputImage_ = new PaintingPanel(0, 0, 1, 1, promptPanel_);
    pages_[page_input_image] = inputImage_;
    
    resultsPanel_ = new PreviewPanel(inputImage_);
    pages_[page_results] = resultsPanel_;

    promptPanel_->setImagePanel(inputImage_);
    
    this->begin();
    for (int i = 0; i < 4; i++) {
        controlNets_[i] = new PaintingPanel(0, 0, 1, 1, promptPanel_, inputImage_, true);
        pages_[page_controlnet1 + i] = controlNets_[i];
    }
    
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

    loadConfig();
    goPage(page_results);
}

Pages::~Pages() {
}

void Pages::loadConfig() {
    for (int i = page_controlnet1; i <= page_controlnet4; i++) {
        visible_pages_[i] = getConfig().getControlnetCount() > i - page_controlnet1;
    }
    alignComponents();
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

    return result;
}

page_t Pages::getPageAtIndex(int index) {
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

int Pages::getIndexAtPage(page_t page) {
    if (!isVisible(page)) {
        return 0;
    }
    int index = 0;
    for (int i = 0; i < page_max; ++i) {
        if (page == i) {
            return index;
        }
        if (isVisible((page_t) i)) 
            ++index;
    }
    return 0;
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

void Pages::setInputImage(RawImage *img, painting_mode_t mode) {
    goPage(page_input_image);
    inputImage_->setImage(img);
    inputImage_->setSelectedMode(mode);
}

RawImage *Pages::getInputImage() {
    return inputImage_->getImage();
}

void Pages::refreshModels() {
    promptPanel_->refreshModels();
}

void Pages::textToImage() {
    if (!promptPanel_->ready(false)) {
        return;
    }
    int seed = promptPanel_->getSeed();
    if (seed == -1) 
        seed = get_sd_state()->randomSeed();

    if (!inputImage_->ready()) {
        return;
    }

    bool is_inpaint = inputImage_->getImg2ImgImage() != NULL && inputImage_->getImg2ImgMask() != NULL;
    const char* model = promptPanel_->getSdModel(is_inpaint);

    if (model == NULL) {
        show_error("Add a model file (.safetensors or .ckpt) into 'models/stable diffusion' directory before you start!");
        return;
    }

    controlnet_list_t controlnets;

    std::shared_ptr c = inputImage_->getControlnet();

    if (c) {
        controlnets.push_back(c);
    }

    for (int i = 0; i < COUNTOF(controlNets_); ++i) {
        c = controlNets_[i]->getControlnet();
        if (c) {
            controlnets.push_back(c);
        }
    }
    
    bool reload = promptPanel_->shouldReload(true);

    std::shared_ptr<GeneratorBase> g;
    if (inputImage_->getImg2ImgImage()) {
        auto imgRaw = inputImage_->getImg2ImgImage();
        auto maskRaw = inputImage_->getImg2ImgMask();
        bool inpaintMasked = inputImage_->shouldInpaintMasked();

        if (!imgRaw) {
            show_error("No input image to proceed!");
            return;
        }

        auto img = imgRaw->duplicate();

        image_ptr_t mask;

        if (maskRaw) {
            if (inpaintMasked) {
                mask = maskRaw->removeAlpha();
            } else {
                mask = maskRaw->duplicate();
            }
        }

        g.reset(new GeneratorImg2Image(
            std::make_shared<SeedGenerator>(),
            false,
            promptPanel_->getPrompt(),
            promptPanel_->getNegativePrompt(),
            get_sd_state()->getSdModelPath(model),
            controlnets,
            img,
            mask,
            seed,
            promptPanel_->getWidth(),
            promptPanel_->getHeight(),
            promptPanel_->getSteps(),
            promptPanel_->getCFG(),
            promptPanel_->getVariationStrength(),
            inputImage_->get_denoise_strength(),
            promptPanel_->shouldRestoreFaces(),
            false,
            reload,
            inputImage_->maskBlurEnabled() ? getConfig().inpaint_get_mask_blur() : 0,
            inputImage_->getInpaintMode()
        ));
    } else {
        g.reset(new GeneratorTxt2Image(
            std::make_shared<SeedGenerator>(),
            false,
            promptPanel_->getPrompt(),
            promptPanel_->getNegativePrompt(),
            get_sd_state()->getSdModelPath(model),
            controlnets,
            seed,
            promptPanel_->getWidth(),
            promptPanel_->getHeight(),
            promptPanel_->getSteps(),
            promptPanel_->getCFG(),
            promptPanel_->getVariationStrength(),
            promptPanel_->shouldRestoreFaces(),
            false,
            reload
        ));
    }

    get_sd_state()->clearGenerators();
    if (!get_sd_state()->generatorAdd(g)) {
        show_error(get_sd_state()->lastError());
    } else {
        resultsPanel_->goLastImage();
    }
}

}   // namespace dexpert
