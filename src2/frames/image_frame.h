#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>

#include "components/image_panel.h"

namespace dfe
{

typedef enum {
   painting_disabled,
   painting_img2img,
   painting_inpaint_masked,
   painting_inpaint_not_masked,
   
   // keep painting_mode_max ath the end
   painting_mode_max
} painting_mode_t;


typedef enum {
   brush_size_disabled,
   brush_size_1,
   brush_size_2,
   brush_size_4,
   brush_size_8,
   brush_size_16,
   brush_size_32,
   brush_size_64,
   brush_size_128,
   // 
   brush_size_count
} brush_size_t;


typedef enum {
    inpaint_original,
    inpaint_fill,
    inpaint_img2img,
    inpaint_wholepicture,
    inpaint_wholefill,
    // inpaint_none,
    // keep inpaint_mode_count at the end
    inpaint_mode_count
} inpaint_mode_t;



class ImageFrame {
public:
    ImageFrame(Fl_Group *parent, ImagePanel *img);
    ~ImageFrame();

    void alignComponents();
private:
    Fl_Group *parent_;
    ImagePanel *img_;
    Fl_Choice *mode_;
    Fl_Choice *brush_size_;
    Fl_Choice *inpaint_mode_;

};

} // namespace dfe
