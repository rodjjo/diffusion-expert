#include "src/windows/console_viewer.h"
#include "src/windows/config_window.h"
#include "src/windows/diffusion_tool.h"
#include "src/python/helpers.h"
#include "src/config/config.h"
#include "src/data/xpm.h"

#include "src/windows/main_window.h"


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

    image_editor_ = new ImagePanel(0, 0, 1, 1);

    initMenubar();

    btn_none_.reset(new Button(xpm::image(xpm::editor_apply), [this] { 
        toolClicked(btn_none_.get());
        image_editor_->setTool(image_tool_none);
    }));
    btn_drag_.reset(new Button(xpm::image(xpm::cursor_drag), [this] { 
        toolClicked(btn_drag_.get());
        image_editor_->setTool(image_tool_drag);
    }));
    btn_drag_float_.reset(new Button(xpm::image(xpm::cursor_drag), [this] { 
        toolClicked(btn_drag_float_.get());
        image_editor_->setTool(image_tool_drag_paste);
    }));
    btn_zoom_.reset(new Button(xpm::image(xpm::lupe_16x16), [this] { 
        toolClicked(btn_zoom_.get());
        image_editor_->setTool(image_tool_zoom);
    }));
    btn_select_.reset(new Button(xpm::image(xpm::cursor_resize), [this] {
        toolClicked(btn_select_.get());
        image_editor_->setTool(image_tool_select);
    }));

    wnd->end();

    btn_none_->position(1, 1);
    btn_drag_->position(1, 1);
    btn_drag_float_->position(1, 1);
    btn_zoom_->position(1, 1);
    btn_select_->position(1, 1);

    btn_none_->tooltip("Disable tools");
    btn_drag_->tooltip("Drag tool");
    btn_drag_float_->tooltip("Drag the floating image");
    btn_zoom_->tooltip("Zoom tool");
    btn_select_->tooltip("Select tool");

    btn_none_->enableDownUp();
    btn_drag_->enableDownUp();
    btn_drag_float_->enableDownUp();
    btn_zoom_->enableDownUp();
    btn_select_->enableDownUp();

    wnd->resizable(wnd);

    this->show();

    alignComponents();
}


void MainWindow::initMenubar() {
    menuPanel_ = new Fl_Group(0, 20, this->w(), 20);
    menuPanel_->end();
    menuPanel_->box(FL_BORDER_BOX);

    initMenu();
}

void MainWindow::initMenu() {
    menuPanel_->begin();
    menu_ = new MainMenu(this->w(), 20, [this] {
        // mainmenu.onclick();
    });
    menuPanel_->end();
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
    menuPanel_->position(0, 0);
    int w = this->w();
    int h = this->h();
    menuPanel_->size(w, menu_->h());
    menu_->position(0, 0);
    menu_->size(w, menuPanel_->h());
    image_editor_->resize(40, menuPanel_->h() + 5, w - 50, h - 15 - menuPanel_->h());

    btn_none_->size(30, 30);
    btn_drag_->size(30, 30);
    btn_drag_float_->size(30, 30);
    btn_zoom_->size(30, 30);
    btn_select_->size(30, 30);

    btn_none_->position(5, menuPanel_->y() + menuPanel_->h() + 2);
    btn_drag_->position(5, btn_none_->y() + btn_none_->h() + 2);
    btn_drag_float_->position(5, btn_drag_->y() + btn_drag_->h() + 2);
    btn_zoom_->position(5, btn_drag_float_->y() + btn_drag_float_->h() + 2);
    btn_select_->position(5, btn_zoom_->y() + btn_zoom_->h() + 2);
}

void MainWindow::editConfig() {
    show_configuration(); 
}

void MainWindow::resize(int x, int y, int w, int h) {
    Fl_Menu_Window::resize(x, y, w, h);
    alignComponents();
}

void MainWindow::toolClicked(Button* btn) {
    btn->down(true);
    Button* buttons[] = {
        btn_none_.get(),
        btn_drag_.get(),
        btn_drag_float_.get(),
        btn_zoom_.get(),
        btn_select_.get(),
    };
    for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); ++i) {
        if (btn != buttons[i]) {
            buttons[i]->down(false);
        }
    }
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
