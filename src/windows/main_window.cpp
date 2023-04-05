#include "src/windows/main_window.h"
#include "src/windows/console_viewer.h"
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

    initLeftPanel();
    initRightPanel();
    initBottomPanel();
    initToolbar();
    
    wnd->end();

    alignComponents();

    wnd->resizable(wnd);

    this->show();

    alignComponents();
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
        page_browser_->add(pages_->pageTitle((page_t) i));
    }
}

void MainWindow::initLeftPanel() {
    leftPanel_ = new Fl_Group(0, 20, 200, this->h() - 120);
    leftPanel_->end();
    leftPanel_->box(FL_BORDER_BOX);
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

    menu_->addItem(noCall, "", "File/New");
    menu_->addItem([this] { pages_->openInputImage(); }, "", "File/Open");
    menu_->addItem([this] { pages_->saveInputImage(); }, "", "File/Save");
    menu_->addItem(noCall, "", "Edit");
    menu_->addItem([this] { pages_->generateInputImage(); }, "", "Run/Generate");
    menu_->addItem(noCall, "", "Tools");
    menu_->addItem(noCall, "", "Help");
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

    int topH = menu_->h();
    int bottomH = 130;
    int leftH = h;
    int rightH = leftH;
    int centerH = h - topH - bottomH;
    int leftW = 100;
    int rightW = 100;
    int centerW = w - leftW - rightW;

    pages_->position(leftW, topH);
    pages_->size(centerW, centerH);

    console_->position(leftW, h - bottomH);
    console_->size(centerW, bottomH);

    leftPanel_->position(0, topH);
    leftPanel_->size(leftW, leftH);

    rightPanel_->position(leftW + centerW, topH);
    rightPanel_->size(leftW, leftH);

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

void MainWindow::installPyDeps(void *cbdata) { 
    ((MainWindow *) cbdata)->installPyDeps();
}

void MainWindow::pageChangeCallback(Fl_Widget* widget, void *cbdata) {
    ((MainWindow *) cbdata)->gotoSelectedPage();
}

void MainWindow::gotoSelectedPage() {
    if (selecting_page_)
        return;
    selecting_page_ = true;
    int idx = page_browser_->value();
    if (idx > 0) 
    {
        page_browser_->deselect();
        page_browser_->select(idx);
        idx -= 1;
        pages_->goPage((page_t) idx);
    }
    selecting_page_ = false;
}

void MainWindow::installPyDeps() {
    bool success = false;
    std::string message = "Unexpected error. Callback did not run!";
    dexpert::runPyShowConsole("Installing Python dependencies", [&success, &message] {
        auto installer = dexpert::py::install_dependencies_helper([&success, &message] (bool st, const char *msg) {
            success = st;
            if (msg) {
                message = msg;
            }
        });
        installer();
    }, [&success, &message] () -> bool {
        if (!success) {
            fl_alert("%s", message.c_str());
        }
        return success;
    });
    
    if (!success) {
        this->hide();
    }
}

int MainWindow::run() {
    Fl::add_timeout(0.101, &MainWindow::installPyDeps, this);
    
    int result = Fl::run();
    return result;
}

}  // namespace dexpert
