#include "src/windows/console_viewer.h"
#include "src/windows/config_window.h"
#include "src/windows/diffusion_tool.h"
#include "src/dialogs/common_dialogs.h"
#include "src/dialogs/size_dialog.h"
#include "src/dialogs/model_downloader.h"
#include "src/python/helpers.h"
#include "src/config/config.h"
#include "src/data/xpm.h"

#include "src/windows/main_window.h"


#define MIN_WIDTH 860
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

    image_editor_ = new ImagePanel(0, 0, 1, 1, [this] {
        updateScrollbar();
    });

    initMenubar();
    
    leftPanel_ = new Fl_Group(0, 0, 1, 1);
    leftPanel_->end();
    bottomPanel_ = new Fl_Group(0, 0, 1, 1);
    bottomPanel_->end();

    leftPanel_->begin();

    btn_none_.reset(new Button("", [this] { 
        toolClicked(btn_none_.get());
        image_editor_->setTool(image_tool_none);
    }));
    btn_drag_.reset(new Button(xpm::image(xpm::drag_icon), [this] { 
        toolClicked(btn_drag_.get());
        image_editor_->setTool(image_tool_drag);
    }));
    btn_zoom_.reset(new Button(xpm::image(xpm::lupe_16x16), [this] { 
        toolClicked(btn_zoom_.get());
        image_editor_->setTool(image_tool_zoom);
    }));
    btn_select_.reset(new Button(xpm::image(xpm::select_icon), [this] {
        toolClicked(btn_select_.get());
        image_editor_->setTool(image_tool_select);
    }));

    leftPanel_->end();

    bottomPanel_->begin();
    
    label_zoom_ = new Fl_Box(0, 0, 1, 1);
    label_select_ = new Fl_Box(0, 0, 1, 1);
    label_scroll_ = new Fl_Box(0, 0, 1, 1);
    label_size_ = new Fl_Box(0, 0, 1, 1);

    bottomPanel_->end();

    wnd->end();

    leftPanel_->box(FL_DOWN_BOX);
    bottomPanel_->box(FL_DOWN_BOX);

    btn_none_->position(1, 1);
    btn_drag_->position(1, 1);
    btn_zoom_->position(1, 1);
    btn_select_->position(1, 1);

    btn_none_->tooltip("Disable tools");
    btn_drag_->tooltip("Drag tool");
    btn_zoom_->tooltip("Zoom tool");
    btn_select_->tooltip("Select tool");

    btn_none_->enableDownUp();
    btn_drag_->enableDownUp();
    btn_zoom_->enableDownUp();
    btn_select_->enableDownUp();
    btn_none_->down(true);

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

    menu_->addItem([this] { newImage(false); }, "", "File/New");
    menu_->addItem([this] { newImage(true); }, "", "File/New art");
    menu_->addItem([this] { openImage(); }, "", "File/Open");
    menu_->addItem([this] { saveImage(); }, "", "File/Save");
    menu_->addItem([this] { image_editor_->close(); }, "", "File/Close");
    menu_->addItem([this] { Fl::delete_widget(this); }, "", "File/Exit");
    menu_->addItem([this] { image_editor_->selectAll(); }, "", "Edit/Select All");
    menu_->addItem([this] { editConfig(); }, "", "Edit/Settings");
    menu_->addItem([this] { editSelection(); }, "", "Selection/Use AI Editor");
    menu_->addItem([this] { image_editor_->pasteImage(); }, "", "Selection/Merge to image");
    menu_->addItem([this] { image_editor_->clearPasteImage(); }, "", "Selection/Discart changes");
    menu_->addItem([this] { image_editor_->cropToSelection(); }, "", "Selection/Crop to Selection");
    menu_->addItem([this] { resizeSelection(); }, "", "Selection/Edit size");
    menu_->addItem([this] { resizeCanvas();  }, "", "Image/Resize Canvas");
    menu_->addItem([this] { resizePicture(); }, "", "Image/Resize Picture");
    menu_->addItem([this] { resizeLeft();  }, "", "Image/Resize direction/Left");
    menu_->addItem([this] { resizeRight(); }, "", "Image/Resize direction/Right");
    menu_->addItem([this] { resizeTop(); }, "", "Image/Resize direction/Top");
    menu_->addItem([this] { resizeBottom(); }, "", "Image/Resize direction/Bottom");
    menu_->addItem([this] { restoreSelectionFace(); }, "", "Image/Restore Face");
    menu_->addItem([this] { upScale(1.5); }, "", "Image/Upscale/1.5x");
    menu_->addItem([this] { upScale(2.0); }, "", "Image/Upscale/2x");
    menu_->addItem([this] { upScale(2.5); }, "", "Image/Upscale/2.5x");
    menu_->addItem([this] { upScale(3.0); }, "", "Image/Upscale/3x");
    menu_->addItem([this] { upScale(3.5); }, "", "Image/Upscale/3.5x");
    menu_->addItem([this] { upScale(4.0); }, "", "Image/Upscale/4x");

    menu_->addItem([this] { get_stable_diffusion_image(); }, "", "Tools/AI Editor");
    menu_->addItem([this] { download_model_from_dialog(); }, "", "Tools/Model downloader");
    menu_->addItem([this] { showConsoles("Console windows", true); }, "", "Tools/Terminal");
    
    //  menu_->addItem(noCall, "", "Help");
}

void MainWindow::alignComponents() {
    menuPanel_->position(0, 0);
    int w = this->w();
    int h = this->h();
    int stabusbar_h = 30;
    menuPanel_->size(w, menu_->h());
    menu_->position(0, 0);
    menu_->size(w, menuPanel_->h());
    image_editor_->resize(40, menuPanel_->h() + 5, w - 50, h - 15 - menuPanel_->h() - stabusbar_h);

    btn_none_->size(30, 30);
    btn_drag_->size(30, 30);
    btn_zoom_->size(30, 30);
    btn_select_->size(30, 30);

    leftPanel_->resize(3, image_editor_->y(), image_editor_->x() - 6, image_editor_->h());
    bottomPanel_->resize(3, image_editor_->y() + image_editor_->h() + 3, w - 10, stabusbar_h);

    btn_none_->position(5, leftPanel_->y() +2);
    btn_drag_->position(5, btn_none_->y() + btn_none_->h() + 2);
    btn_zoom_->position(5, btn_drag_->y() + btn_drag_->h() + 2);
    btn_select_->position(5, btn_zoom_->y() + btn_zoom_->h() + 2);

    label_size_->resize(bottomPanel_->x() + 5, bottomPanel_->y() + 2, 200, stabusbar_h - 4);
    label_zoom_->resize(label_size_->x() + label_size_->w() + 2, label_size_->y(), 200, stabusbar_h - 4);
    label_select_->resize(label_zoom_->x() + label_zoom_->w() + 2, label_size_->y(), 200, stabusbar_h - 4);
    label_scroll_->resize(label_select_->x() + label_select_->w() + 2, label_size_->y(), 200, stabusbar_h - 4);
}

void MainWindow::editConfig() {
    show_configuration(); 
}

void MainWindow::newImage(bool fromStableDiffusion) {
    if (fromStableDiffusion) {
        auto img = get_stable_diffusion_image();
        if (img) {
            image_editor_->close();
            image_editor_->setLayerImage(image_type_image, img);
        }
    } else {
        int szx = 512, szy = 512;
        if (getSizeFromDialog("Size of the new image", &szx, &szy)) {
            image_editor_->newImage(szx, szy);
        }
    }
}

void MainWindow::openImage() {
    image_editor_->open(image_type_image);
}

void MainWindow::saveImage() {
    image_editor_->save(image_type_image);
}

void MainWindow::upScale(float scale) {
    image_editor_->upScale(scale);
}

void MainWindow::restoreSelectionFace() {
    image_editor_->restoreSelectionFace();
}

void MainWindow::resizeSelection() {
    int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    image_editor_->getSelection(&x1, &y1, &x2, &y2);
    if (x1 == x2 && y1 == y2) {
        show_error("No selection");
        return;
    }
    int w = x2 - x1;
    int h = y2 - y1;
    if (getSizeFromDialog("Resize the selection area", &w, &h)) {
        image_editor_->resizeSelection(w, h);
    }
}

void MainWindow::resizeLeft() {
    int size = 128;
    if (getSizeFromDialog("Resize the left of the image:", &size)) {
        image_editor_->resizeLeft(size);
    }
}

void MainWindow::resizeRight() {
    int size = 128;
    if (getSizeFromDialog("Resize the right of the image:", &size)) {
        image_editor_->resizeRight(size);
    }
}

void MainWindow::resizeTop() {
    int size = 128;
    if (getSizeFromDialog("Resize the top of the image:", &size)) {
        image_editor_->resizeTop(size);
    }
}

void MainWindow::resizeBottom() {
    int size = 128;
    if (getSizeFromDialog("Resize the bottom of the image:", &size)) {
        image_editor_->resizeBottom(size);
    }
}

void MainWindow::resizePicture() {
    auto img = image_editor_->getLayerImage(image_type_image);
    if (!img) {
        show_error("No image to resize. Open or create one!");
        return;
    }
    int szx = img->w(), szy = img->h();
    if (getSizeFromDialog("Size of the image", &szx, &szy)) {
        image_editor_->resizeImages(szx, szy);
    }
}

void MainWindow::resizeCanvas() {
    auto img = image_editor_->getLayerImage(image_type_image);
    if (!img) {
        show_error("No image to resize. Open or create one!");
        return;
    }
    int szx = img->w(), szy = img->h();
    if (getSizeFromDialog("Size of the image", &szx, &szy)) {
        image_editor_->resizeCanvas(szx, szy);
    }
}

void MainWindow::editSelection() {
    auto selection = image_editor_->getSelectedImage(image_type_image);
    if (selection) {
        auto img = get_stable_diffusion_image(selection.get());
        if (img) {
            image_editor_->setPasteImageAtSelection(image_type_image, img.get());
        }
    } else {
        show_error("Invalid selection. No image to process.");
    }
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

void MainWindow::updateScrollbar() {
    char buffer[512] = {0,};
    coordinate_t rs = image_editor_->getReferenceSize();
    sprintf(buffer, "Dimensions: [%d x %d]", rs.x, rs.y);
    label_size_->copy_label(buffer);
    sprintf(buffer, "Zoom: %0.0f%%", image_editor_->getZoomLevel() * 100);
    //sprintf(buffer, "Zoom: %0.1f Scroll [%d, %d]", image_editor_->getZoomLevel() * 100, image_editor_->getScrollX(), image_editor_->getScrollY());
    label_zoom_->copy_label(buffer);
    int sx1 = 0, sx2 = 0, sy1 = 0, sy2 = 0;
    image_editor_->getSelection(&sx1, &sy1, &sx2, &sy2);
    if (sx1 != sx2 && sy1 != sy2) {
        sprintf(buffer, "Sel: (%d,%d, %d, %d) %d x %d", sx1, sy1, sx2, sy2, sx2 - sx1, sy2 - sy1);
    } else {
        sprintf(buffer, "Sel: no selection");
    }
    label_select_->copy_label(buffer);
    image_editor_->getMouseXY(&sx1, &sy1);
    sprintf(buffer, "Mouse: %d x %d ", sx1, sy1);
    label_scroll_->copy_label(buffer);
}

}  // namespace dexpert
