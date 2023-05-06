/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_WINDOWS_DIFFUSION_TOOL_H_
#define SRC_WINDOWS_DIFFUSION_TOOL_H_

#include <memory>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>

#include "src/controls/button.h"
#include "src/panels/pages.h"
#include "src/windows/main_menu.h"
#include "src/console/console_tabs.h"
#include "src/python/wrapper.h"

namespace dexpert {

class DiffusionTool : public Fl_Window {
 public:
    DiffusionTool();
    ~DiffusionTool();
    image_ptr_t run();
    void setInitialImage(RawImage *image);
    RawImage *getInputImage();

 protected:
   void resize(int x, int y, int w, int h) override;
   int handle(int event) override;
   
 private:
  void initRightPanel();
  void initToolbar();
  void initPagesPanel();
  void alignComponents();
  void gotoSelectedPage();
  void refreshBrowser();
  static void pageChangeCallback(Fl_Widget* widget, void *cbdata);
  static void gotoPromptPage(void *cbdata);

 private:
    Fl_Group  *rightPanel_ = NULL;
    Fl_Group  *toolsPanel_ = NULL;
    Pages *pages_ = NULL;
    Fl_Select_Browser *page_browser_;
    bool confirmed_ = false;
    bool selecting_page_ = false;
    std::unique_ptr<Button> cancelBtn_;
    std::unique_ptr<Button> confirmBtn_;
    std::unique_ptr<Button> consoleBtn_;
};

image_ptr_t get_stable_diffusion_image(RawImage *image);
image_ptr_t get_stable_diffusion_image();

} // namespace dexpert

#endif  // SRC_WINDOWS_DIFFUSION_TOOL_H_