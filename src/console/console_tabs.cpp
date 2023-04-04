#include "src/data/xpm.h"
#include "src/console/console_tabs.h"
#include "src/windows/console_viewer.h"


namespace dexpert
{

ConsoleTabs::ConsoleTabs(int x, int y, int w, int h) : Fl_Group(x, y, w, h, "") {
    this->begin();
    
    tabs_ = new  Fl_Tabs(0, 1, 1, 1);
    tabs_->selection_color(FL_DARK3);
    tabs_->begin();
    
    tabStdout_  = new Fl_Group(0, 1, 1, 1, "Console - stdout");

    stdout_ = new Console(0, 0, 1, 1, get_terminal(log_source_stdout));
    btnCopyStdout_.reset(new Button(xpm::image(xpm::button_scissor), [this] {
        get_terminal(log_source_stdout)->copy_to_clipboard();
    }));
    btnSeeStdout_.reset(new Button(xpm::image(xpm::button_seek), [this] {
        showConsoles("Console viewer", true);
    }));

    tabStdout_->end();

    tabStderr_   = new Fl_Group(0, 1, 1, 1, "Console - stderr");

    stderr_ = new Console(0, 0, 1, 1, get_terminal(log_source_stderr));
    btnCopyStderr_.reset(new Button(xpm::image(xpm::button_scissor), [this] {
        get_terminal(log_source_stderr)->copy_to_clipboard();
    }));
    btnSeeStderr_.reset(new Button(xpm::image(xpm::button_seek), [this] {
        showConsoles("Console viewer", false);
    }));


    tabStderr_->end();
   
    tabs_->end();

    this->end();

    align_components();
}

ConsoleTabs::~ConsoleTabs() {
}

void ConsoleTabs::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    align_components();
}

void ConsoleTabs::align_components() {
    int xx = x() + 2;
    int yy = y() + 2;
    int hh = h() - 4;
    int ww = w() - 4;

    tabs_->position(xx, yy);
    tabs_->size(ww, hh);

    int tabrx = xx + 2;
    int tabry = yy + 2;
    int tabrw = ww - 2;
    int tabrh = hh - 32;
    
    tabStdout_->position(tabrx, tabry);
    tabStdout_->size(tabrw, tabrh);

    tabStderr_ ->position(tabrx, tabry);
    tabStderr_ ->size(tabrw, tabrh);

    int stdw = tabrw - 34;
    
    stdout_->position(tabrx, tabry);
    stdout_->size(stdw, tabrh);
    
    stderr_->position(tabrx, tabry);
    stderr_->size(stdw, tabrh);

    btnCopyStderr_->position(tabrx + stdw + 1 , tabry + 2);
    btnCopyStdout_->position(tabrx + stdw + 1, tabry + 2);
    btnCopyStderr_->size(30, 30);
    btnCopyStdout_->size(30, 30);

    btnSeeStderr_->position(tabrx + stdw + 1 , tabry + 34);
    btnSeeStdout_->position(tabrx + stdw + 1, tabry + 34);
    btnSeeStderr_->size(30, 30);
    btnSeeStdout_->size(30, 30);

    btnCopyStderr_->tooltip("Copy to the clipboard.");
    btnCopyStdout_->tooltip("Copy to the clipboard.");
    btnSeeStderr_->tooltip("See all the contents.");
    btnSeeStdout_->tooltip("See all the contents.");
}

void ConsoleTabs::hideShowConsoleButtons() {
    btnSeeStderr_->hide();
    btnSeeStdout_->hide();
}

void ConsoleTabs::goStdout() {
    tabs_->value(tabStdout_);
}

void ConsoleTabs::goStderr() {
    tabs_->value(tabStderr_);
}


} // namespace dexpert
