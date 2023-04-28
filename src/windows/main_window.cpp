#include "src/windows/main_window.h"
#include "src/windows/console_viewer.h"
#include "src/windows/config_window.h"
#include "src/windows/diffusion_tool.h"
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
    auto wnd = this;

    wnd->size_range(MIN_WIDTH, MIN_HEIGHT);

    wnd->begin();

    initToolbar();

    image_editor_ = new ImagePanel(0, 0, 1, 1);

    wnd->end();

    alignComponents();

    wnd->resizable(wnd);

    this->show();

    alignComponents();
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
    menu_->addItem([this] {
        image_editor_->open(image_type_image);
    }, "", "File/Open");
    menu_->addItem(noCall, "", "File/Save");
    menu_->addItem(noCall, "", "Edit");
    menu_->addItem([this] { get_stable_diffusion_image(); }, "", "Run/Generate");
    menu_->addItem([this] { editConfig(); }, "", "Edit/Settings");
    // menu_->addItem(noCall, "", "Tools");
    //  menu_->addItem(noCall, "", "Help");
}

void MainWindow::alignComponents() {
    toolsPanel_->position(0, 0);
    int w = this->w();
    int h = this->h();
    toolsPanel_->size(w, menu_->h());
    menu_->position(0, 0);
    menu_->size(w, toolsPanel_->h());
    image_editor_->resize(120, toolsPanel_->h() + 5, w - 125, h - 15 - toolsPanel_->h());
}

void MainWindow::editConfig() {
    show_configuration(); 
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

int MainWindow::run() {
    int result = Fl::run();
    return result;
}

}  // namespace dexpert
