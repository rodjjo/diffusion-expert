#ifndef SRC_DIALOG_SIZE_DIALOG_H
#define SRC_DIALOG_SIZE_DIALOG_H

#include <memory>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Int_Input.H>

#include "src/controls/button.h"


namespace dexpert
{

class SizeWindow : public Fl_Window  {
 public:
    SizeWindow(const char *title);
    virtual ~SizeWindow();
    void setInitialSize(int x, int y);
    void retriveSize(int *x, int *y);
    bool run();
 private:
    void confirmOk();
 private:
    bool ok_confirmed_ = false;
    Fl_Int_Input *width_;
    Fl_Int_Input *height_;
    std::unique_ptr<Button> btn_ok_;
    std::unique_ptr<Button> btn_cancel_;
};

bool getSizeFromDialog(const char *title, int *x, int *y);
    
} // namespace dexpert


#endif // SRC_DIALOG_SIZE_DIALOG_H