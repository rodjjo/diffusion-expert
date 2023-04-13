#include "src/windows/main_window.h"
#include "src/windows/console_viewer.h"
#include "src/windows/config_window.h"
#include "src/python/helpers.h"
#include "src/config/config.h"


#define MIN_WIDTH 640
#define MIN_HEIGHT 480

namespace dexpert {

MainWindow::~MainWindow() {
}

MainWindow::MainWindow():  Fl_Menu_Window(
    getConfig().windowXPos(),
    getConfig().windowYPos(),
    getConfig().windowWidth(),
    getConfig().windowHeight(),
    "Stable Diffusion Expert - Py and C++"
) {
    python_ = dexpert::py::get_py();
    
    auto wnd = this;

    wnd->size_range(MIN_WIDTH, MIN_HEIGHT);

    wnd->begin();

    initPagesPanel();
    initRightPanel();
    initBottomPanel();
    initToolbar();

    wnd->end();

    alignComponents();

    wnd->resizable(wnd);

    this->show();

    alignComponents();

    Fl::add_timeout(0.01, &MainWindow::gotoPromptPage, this);
}

void MainWindow::initBottomPanel() {
    console_ = new ConsoleTabs(0, 0, 1, 1);
    console_->end();
}

void MainWindow::initRightPanel() {
    rightPanel_ = new Fl_Group(this->w() - 200, 20, 200, this->h() - 120);
    rightPanel_->begin();
    page_browser_ = new Fl_Select_Browser(0, 0, 1, 1);
    page_browser_->callback(pageChangeCallback, this);
    rightPanel_->end();
    rightPanel_->box(FL_BORDER_BOX);
    for (int i = 0; i < page_max; ++i) {
        if (pages_->isVisible((page_t) i))
            page_browser_->add(pages_->pageTitle((page_t) i));
    }
}

void MainWindow::initToolbar() {
    toolsPanel_ = new Fl_Group(0, 20, this->w(), 20);
    toolsPanel_->end();
    toolsPanel_->box(FL_BORDER_BOX);

    initMenu();
}

void MainWindow::initMenu() {
    toolsPanel_->begin();
    menu_ = new MainMenu(this->w(), 20, [this] {
        // mainmenu.onclick();
    });
    toolsPanel_->end();
    callback_t noCall = []{};

    // menu_->addItem(noCall, "", "File/New");
    // menu_->addItem([this] {  }, "", "File/Open");
    // menu_->addItem([this] {  }, "", "File/Save");
    //  menu_->addItem(noCall, "", "Edit");
    menu_->addItem([this] { pages_->textToImage(); }, "", "Run/Generate");
    menu_->addItem([this] { show_configuration(); }, "", "Edit/Settings");
    // menu_->addItem(noCall, "", "Tools");
    //  menu_->addItem(noCall, "", "Help");
}

void MainWindow::initPagesPanel() {
    pages_ = new Pages(200, 20, this->w() - 400, this->h()-120);
    pages_->end();
}

void MainWindow::alignComponents() {
    toolsPanel_->position(0, 0);
    int w = this->w();
    int h = this->h();
    toolsPanel_->size(w, menu_->h());
    menu_->position(0, 0);
    menu_->size(w, toolsPanel_->h());

    int topH = menu_->h() + 2;
    int bottomH = 130;
    int leftH = h;
    int rightH = leftH;
    int centerH = h - topH - bottomH;
    int leftW = 0;
    int rightW = 100;
    int centerW = w - leftW - rightW;

    pages_->position(leftW, topH);
    pages_->size(centerW, centerH);

    console_->position(leftW, h - bottomH);
    console_->size(w, bottomH);

    rightPanel_->position(leftW + centerW, topH);
    rightPanel_->size(rightW, centerH);

    page_browser_->resize(rightPanel_->x(), rightPanel_->y(), rightPanel_->w(), rightPanel_->h());
}

void MainWindow::resize(int x, int y, int w, int h) {
    Fl_Menu_Window::resize(x, y, w, h);
    alignComponents();
}

int MainWindow::handle(int event) {
    switch (event) {
        case FL_KEYUP: {
            if (Fl::event_key() == FL_Escape) {
                return  1;
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

    return Fl_Menu_Window::handle(event);
}

void MainWindow::pageChangeCallback(Fl_Widget* widget, void *cbdata) {
    ((MainWindow *) cbdata)->gotoSelectedPage();
}

void MainWindow::gotoPromptPage(void *cbdata) {
    ((MainWindow*) cbdata)->page_browser_->select(1);
    ((MainWindow*) cbdata)->pages_->goPage(page_prompts);
}

void MainWindow::gotoSelectedPage() {
    if (selecting_page_)
        return;
    selecting_page_ = true;
    int idx = page_browser_->value();
    if (idx > 0)  {
        idx -= 1;
        pages_->goPage(pages_->getPageIndex(idx));
    } 
    page_browser_->deselect();
    page_browser_->select(pages_->visibleIndex() + 1);
    selecting_page_ = false;
}

int MainWindow::run() {
    int result = Fl::run();
    return result;
}

}  // namespace dexpert
