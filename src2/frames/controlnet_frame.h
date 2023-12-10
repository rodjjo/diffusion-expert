#pragma once

#include <memory>

#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>

#include "components/image_panel.h"
#include "components/button.h"

namespace dfe
{


typedef enum {
   controlnet_disabled,
   controlnet_scribble,
   controlnet_canny,
   controlnet_pose,
   controlnet_deepth,
   controlnet_segmentation,
   controlnet_lineart,
   controlnet_mangaline,

   //
   controlnet_mode_max
} controlnet_mode_t;

class ControlnetFrame {
public:
    ControlnetFrame(Fl_Group *parent, ImagePanel *img);
    ~ControlnetFrame();

    void alignComponents();
private:
    Fl_Group *parent_;
    ImagePanel *img_;
    Fl_Choice *mode_;
    std::unique_ptr<Button> btnPreprocess_;
    std::unique_ptr<Button> btnOpenMask_;
    std::unique_ptr<Button> btnSaveMask_;
};

} // namespace dfe
