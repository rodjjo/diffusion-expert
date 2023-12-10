#include "src/windows/console_viewer.h"
#include "src/windows/config_window.h"
#include "src/python/helpers.h"
#include "src/config/config.h"
#include "src/stable_diffusion/state.h"
#include "src/dialogs/common_dialogs.h"

#include "src/windows/diffusion_tool.h"


#define MIN_WIDTH 640
#define MIN_HEIGHT 740

namespace dexpert {

DiffusionTool::~DiffusionTool() {
}

DiffusionTool::DiffusionTool():  Fl_Window(
    getConfig().windowXPos(),
    getConfig().windowYPos(),
    getConfig().windowWidth(),
    getConfig().windowHeight(),
    "Image editor - Stable Diffusion"
) {
    auto wnd = this;
    wnd->size_range(MIN_WIDTH, MIN_HEIGHT);


    wnd->begin();

    initPagesPanel();
    
    initRightPanel();
    
    initToolbar();

    wnd->end();

    wnd->resizable(wnd);

    alignComponents();
    
    wnd->show();
    wnd->set_modal();
}

void DiffusionTool::initRightPanel() {
    rightPanel_ = new Fl_Group(this->w() - 205, 20, 200, this->h() - 120);
    rightPanel_->begin();
    generateBtn_.reset(new Button(xpm::image(xpm::button_play), [this] {
        pages_->textToImage();
    }));
    page_browser_ = new Fl_Select_Browser(0, 0, 1, 1);
    page_browser_->callback(pageChangeCallback, this);
    rightPanel_->end();
    rightPanel_->box(FL_BORDER_BOX);
    pages_->goPage(page_prompts);
    refreshBrowser();
    pages_->end();
    generateBtn_->tooltip("Generate a new image. [shortcut key: F9]");
}

void DiffusionTool::refreshBrowser() {
    page_browser_->clear();
    for (int i = 0; i < page_max; ++i) {
        if (pages_->isVisible((page_t) i))
            page_browser_->add(pages_->pageTitle((page_t) i));
    }
    page_browser_->deselect();
    page_browser_->select(pages_->visibleIndex() + 1);
}

void DiffusionTool::initToolbar() {
    toolsPanel_ = new Fl_Group(0, 20, this->w(), 20);
    cancelBtn_.reset(new Button(xpm::image(xpm::button_cancel_16x16), [this] {
       confirmed_ = false;
       this->hide();
    }));
    confirmBtn_.reset(new Button(xpm::image(xpm::button_ok_16x16), [this] {
         if (pages_->getInputImage()) {
            confirmed_ = true;
            this->hide();
        } else {
            show_error("Generate an image and select it first!");
        }
    }));
    consoleBtn_.reset(new Button(xpm::image(xpm::lupe_16x16), [this] {
       showConsoles("Console windows", true);
    }));

    toolsPanel_->end();
    toolsPanel_->box(FL_BORDER_BOX);
    cancelBtn_->tooltip("Discart all changes and close the window");
    confirmBtn_->tooltip("Confirm");
    consoleBtn_->tooltip("Show the console window");
}

void DiffusionTool::initPagesPanel() {
    pages_ = new Pages(200, 20, this->w() - 400, this->h()-120);
    pages_->end();
}

void DiffusionTool::alignComponents() {
    int w = this->w();
    int h = this->h();

    int topH = 2;
    int toolbarH = 60;
    int bottomH = 5;
    int centerH = h - topH - bottomH - toolbarH;
    int leftW = 0;
    int rightW = 105;
    int centerW = w - leftW - rightW;

    pages_->position(leftW, topH);
    pages_->size(centerW, centerH);

    rightPanel_->position(leftW + centerW, topH);
    rightPanel_->size(rightW - 5, centerH);

    page_browser_->resize(rightPanel_->x(), rightPanel_->y() + 60, rightPanel_->w(), rightPanel_->h() - 60);
    
    toolsPanel_->position(0, h - toolbarH);
    toolsPanel_->size(w, toolbarH);
    cancelBtn_->size(200, 40);
    confirmBtn_->size(200, 40);
    consoleBtn_->size(30, 30);
    
    confirmBtn_->position(toolsPanel_->x() + toolsPanel_->w() - confirmBtn_->w() * 2 - 7, toolsPanel_->y() + 5);
    cancelBtn_->position(toolsPanel_->x() + toolsPanel_->w() - confirmBtn_->w() - 5, toolsPanel_->y() + 5);

    consoleBtn_->position(5, toolsPanel_->y() + toolsPanel_->h() / 2 - consoleBtn_->h() / 2);

    generateBtn_->position(page_browser_->x() + page_browser_->w() / 2 - 25, page_browser_->y() - 55);
    generateBtn_->size(50, 50);
    
}

void DiffusionTool::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
    alignComponents();
}

int DiffusionTool::handle(int event) {
    switch (event) {
        case FL_KEYUP: {
            if (Fl::event_key() == FL_Escape) {
                return 1;
            }
            if (Fl::event_key() == FL_F + 9) {
                pages_->textToImage();
                return 1;
            }
        }
        break;
        case FL_KEYDOWN: {
            if (Fl::event_key() == FL_Escape) {
                return 1;
            }
        }
        break;
    }

    return Fl_Window::handle(event);
}

void DiffusionTool::pageChangeCallback(Fl_Widget* widget, void *cbdata) {
    ((DiffusionTool *) cbdata)->gotoSelectedPage();
}

void DiffusionTool::gotoSelectedPage() {
    if (selecting_page_)
        return;
    selecting_page_ = true;
    int idx = page_browser_->value();
    if (idx > 0)  {
        idx -= 1;
        pages_->goPage(pages_->getPageAtIndex(idx));
    } 
    page_browser_->deselect();
    page_browser_->select(pages_->visibleIndex() + 1);
    selecting_page_ = false;
}

image_ptr_t DiffusionTool::run() {
    this->pages_->refreshModels();
    this->show();
    while (this->shown()) {
        Fl::wait(0.001);
    }
    RawImage *img = confirmed_ ? pages_->getInputImage() : NULL;
    if (img) {
        return img->duplicate();
    }
    return image_ptr_t();
}

void DiffusionTool::setInitialImage(RawImage *image, painting_mode_t mode) {
    if (!image) {
        return;
    }
    pages_->setInputImage(image, mode);
    page_browser_->value(pages_->getIndexAtPage(pages_->activePage()) + 1);
    gotoSelectedPage();
}

RawImage *DiffusionTool::getInputImage() {
    return pages_->getInputImage();
}

image_ptr_t get_stable_diffusion_image(RawImage *image, painting_mode_t mode) {

    get_sd_state()->clearGenerators();
    DiffusionTool *window = new DiffusionTool();

    window->setInitialImage(image, mode);

    image_ptr_t r = window->run();
    Fl::delete_widget(window);
    Fl::do_widget_deletion();
    return r;
}

image_ptr_t get_stable_diffusion_image() {
    return get_stable_diffusion_image(NULL, paiting_disabled);
}

}  // namespace dexpert
