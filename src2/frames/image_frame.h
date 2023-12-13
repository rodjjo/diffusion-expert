#pragma once

#include <memory>
#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>

#include "components/button.h"
#include "components/image_panel.h"

namespace dfe
{

typedef enum {
   img2img_disabled,
   img2img_img2img,
   img2img_inpaint_masked,
   img2img_inpaint_not_masked,
   
   // keep img2img_mode_max ath the end
   img2img_mode_max
} img2img_mode_t;


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
    bool enabled();
    img2img_mode_t get_mode();
    int get_brush_size();
    inpaint_mode_t get_inpaint_mode();
    void enable_mode();

private:
    void combobox_selected();
protected:
    static void combobox_cb(Fl_Widget* widget, void *cbdata);
    void combobox_cb(Fl_Widget* widget);

private:
    bool inside_cb_ = false;


private:
    Fl_Group *parent_;
    ImagePanel *img_;
    Fl_Choice *choice_mode_;
    Fl_Choice *choice_brush_size_;
    Fl_Choice *choice_inpaint_mode_;
    std::unique_ptr<Button> btnNewMask_;
    std::unique_ptr<Button> btnOpenMask_;

    img2img_mode_t mode_ = img2img_disabled;
    int brush_size_ = 16;
    inpaint_mode_t inpaint_mode_ = inpaint_original;

};

} // namespace dfe
