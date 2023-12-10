#include "frames/image_frame.h"

namespace dfe
{
    const char *modes_text[painting_mode_max] = {
        "Disabled",
        "Img2Img",
        "Inpaint masked",
        "Inpaint not masked"
    };

    const char *brush_captions[brush_size_count] = {
        "Disabled",
        "1 Pixel",
        "2 Pixels",
        "4 Pixels",
        "8 Pixels",
        "16 Pixels",
        "32 Pixels",
        "64 Pixels",
        "128 Pixels"
    };

    const uint8_t brushes_sizes[brush_size_count] = {
        0, 1, 2, 4, 8, 16, 32, 64, 128
    };

    const char *inpaint_modes[inpaint_mode_count] = {
        "Original image",
        "Fill image",
        "Use Img2Image",
        "Whole image (original)",
        "Whole image (fill)",
        // "Latent Noise",
        // "Latent Nothing"
    };


    
ImageFrame::ImageFrame(Fl_Group *parent, ImagePanel *img) {
    parent_ = parent;
    img_ = img;
    auto current = parent->current();

    mode_ = new Fl_Choice(0 , 0, 1, 1, "Image usage mode");
    brush_size_ = new Fl_Choice(0 , 0, 1, 1, "Brush size");;
    inpaint_mode_ = new Fl_Choice(0 , 0, 1, 1, "Inpainting mode");;

    for (int i = 0; i < painting_mode_max; i++) {
        mode_->add(modes_text[i]);
    }
    for (int i = 0; i < brush_size_count; i++) {
        brush_size_->add(brush_captions[i]);
    }
    for (int i = 0; i < inpaint_mode_count; i++) {
        inpaint_mode_->add(inpaint_modes[i]);
    }

    mode_->align(FL_ALIGN_TOP_LEFT);
    brush_size_->align(FL_ALIGN_TOP_LEFT);
    inpaint_mode_->align(FL_ALIGN_TOP_LEFT);

    mode_->value(0);
    brush_size_->value(0);
    inpaint_mode_->value(0);

    alignComponents();
}

ImageFrame::~ImageFrame() {
}

void ImageFrame::alignComponents() {
    int left = parent_->x();
    int top = parent_->y();
    int w = img_->x() - parent_->x();
    int h = parent_->h();

    mode_->resize(left + 5, top + 25, w - 10, 30);
    inpaint_mode_->resize(left + 5, mode_->h() + mode_->y() + 25, w - 10, 30);
    brush_size_->resize(left + 5, inpaint_mode_->h() + inpaint_mode_->y() + 25, w - 10, 30);
}

} // namespace dfe
