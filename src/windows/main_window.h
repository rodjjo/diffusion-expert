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

#include "src/panels/painting_panel.h"
#include "src/windows/main_menu.h"
#include "src/python/wrapper.h"
#include "src/controls/image_panel.h"
#include "src/controls/button.h"

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
  void initMenubar();
  void initMenu();
  void alignComponents();
  void editConfig();
  void updateStatusbar();
  void newImage(bool fromStableDiffusion);
  void openImage();
  void saveImage();
  void editSelection(painting_mode_t mode);
  void resizeLeft();
  void resizeRight();
  void resizeTop();
  void resizeBottom(); 
  void resizePicture();
  void resizeCanvas();
  void resizeSelection(int width);
  void restoreSelectionFace();
  void removeSelectionBackground();
  void upScale(float scale);
  void flip(bool vertical);
  void closeImage();

 private:
    Fl_Group *menuPanel_ = NULL;
    Fl_Group *bottomPanel_ = NULL;
    Fl_Box * label_zoom_ = NULL;
    Fl_Box * label_select_ = NULL;
    Fl_Box * label_scroll_ = NULL;
    Fl_Box * label_size_ = NULL;
    MainMenu *menu_ = NULL;
    ImagePanel *image_editor_;
};


} // namespace dexpert

#endif  // SRC_WINDOWS_MAIN_WINDOW_H_