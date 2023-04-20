/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PAGES_H_
#define SRC_PANELS_PAGES_H_

#include <string>

#include <FL/Fl_Group.H>

#include "src/panels/painting_panel.h"
#include "src/panels/prompt_panel.h"
#include "src/panels/results_panel.h"

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
    page_t getPageIndex(int index);
    page_t activePage();
    const char *pageTitle(page_t page);
    void textToImage();
    void loadConfig();

 protected:
    void resize(int x, int y, int w, int h) override;

 private:
    void alignComponents();

 private:
    page_t active_page_ = page_prompts;
    Fl_Group *pages_[page_max] = {0,};
    bool visible_pages_[page_max] = {0,};

    PromptPanel *promptPanel_ = NULL;
    PaintingPanel *inputImage_ = NULL;
    ResultsPanel *resultsPanel_ = NULL;
    std::string current_open_input_dir_;
};
    
} // namespace dexpert


#endif  // SRC_PANELS_PAGES_H_