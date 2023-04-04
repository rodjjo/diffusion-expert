/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_WINDOWS_MAIN_WINDOW_H_
#define SRC_WINDOWS_MAIN_WINDOW_H_

#include <memory>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Tabs.H>

#include "src/panels/pages.h"
#include "src/windows/main_menu.h"
#include "src/console/console_tabs.h"
#include "src/python/wrapper.h"

namespace dexpert {

class MainWindow : Fl_Menu_Window {
 public:
    MainWindow();
    ~MainWindow();
    int run();
 protected:
   void resize(int x, int y, int w, int h) override;
   int handle(int event) override;
   
 private:
  void initBottomPanel();
  void initRightPanel();
  void initLeftPanel();
  void initToolbar();
  void initPagesPanel();
  void initMenu();
  void alignComponents();
  void gotoSelectedPage();
  static void pageChangeCallback(Fl_Widget* widget, void *cbdata);

 private:
    Fl_Group  *leftPanel_ = NULL;
    Fl_Group  *rightPanel_ = NULL;
    Fl_Group  *toolsPanel_ = NULL;
    ConsoleTabs *console_ = NULL;
    Pages *pages_ = NULL;
    MainMenu *menu_ = NULL;
    Fl_Select_Browser *page_browser_;
    bool selecting_page_ = false;
    std::shared_ptr<dexpert::py::PythonMachine> python_;
};


} // namespace dexpert

#endif  // SRC_WINDOWS_MAIN_WINDOW_H_