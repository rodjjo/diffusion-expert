/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PAGES_H_
#define SRC_PANELS_PAGES_H_

#include <string>

#include <FL/Fl_Group.H>

#include "src/panels/painting_panel.h"
#include "src/panels/prompt_panel.h"
#include "src/panels/preview_panel.h"
#include "src/python/raw_image.h"

namespace dexpert
{

typedef enum {
    page_prompts,
    page_input_image,
    page_controlnet1,
    page_controlnet2,
    page_controlnet3,
    page_controlnet4,
    page_results,
    // keep page_max at the end
    page_max
} page_t;

class Pages: public Fl_Group {
 public:
    Pages(int x, int y, int w, int h);
    virtual ~Pages();
    bool isVisible(page_t page);
    int pageCount();
    bool goPage(page_t page);
    int visibleIndex();
    page_t getPageAtIndex(int index);
    int getIndexAtPage(page_t page);
    page_t activePage();

    const char *pageTitle(page_t page);
    void textToImage();
    void loadConfig();
    void setInputImage(RawImage *img, painting_mode_t mode);
    RawImage *getInputImage();
    void refreshModels();
    
 protected:
    void resize(int x, int y, int w, int h) override;

 private:
    void alignComponents();

 private:
    page_t active_page_ = page_prompts;
    Fl_Group *pages_[page_max] = {0,};
    bool visible_pages_[page_max] = {0,};

    PaintingPanel *controlNets_[4] = {};
    PromptPanel *promptPanel_ = NULL;
    PaintingPanel *inputImage_ = NULL;
    PreviewPanel *resultsPanel_ = NULL;
    std::string current_open_input_dir_;
};
    
} // namespace dexpert


#endif  // SRC_PANELS_PAGES_H_