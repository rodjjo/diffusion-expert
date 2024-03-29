#pragma once

#include <memory>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>

#include "components/image_panel.h"
#include "components/button.h"

namespace dfe
{


class ControlnetFrame {
public:
    ControlnetFrame(Fl_Group *parent, ImagePanel *img, ImagePanel *reference);
    ~ControlnetFrame();

    bool enabled();
    void alignComponents();
    std::string getModeStr();
    image_ptr_t getImage();
    
private:
    void pre_process();
    void open_mask();
    void save_mask();
    void load_modes();

protected:
    static void combobox_cb(Fl_Widget* widget, void *cbdata);
    void combobox_cb(Fl_Widget* widget);

private:
    bool inside_cb_ = false;

private:
    Fl_Group *parent_;
    ImagePanel *img_;
    ImagePanel *reference_;
    Fl_Choice *mode_;
    std::unique_ptr<Button> btnPreprocess_;
    std::unique_ptr<Button> btnOpenMask_;
    std::unique_ptr<Button> btnSaveMask_;
};

} // namespace dfe
