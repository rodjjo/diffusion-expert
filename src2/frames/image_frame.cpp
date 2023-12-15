#include "components/xpm/xpm.h"

#include "messagebus/messagebus.h"
#include "frames/image_frame.h"

namespace dfe
{
    const char *modes_text[img2img_mode_max] = {
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

    choice_mode_ = new Fl_Choice(0 , 0, 1, 1, "Image usage mode");
    choice_brush_size_ = new Fl_Choice(0 , 0, 1, 1, "Editor brush size");
    choice_inpaint_mode_ = new Fl_Choice(0 , 0, 1, 1, "Inpainting mode");
    strength_input_ = new Fl_Float_Input(0 , 0, 1, 1, "Similarity");

    btnNewMask_.reset(new Button(xpm::image(xpm::img_24x24_new_document),
        [this] () {
            printf("clicked new mask\n");
            publish_event(this, event_image_frame_new_mask, NULL);
        }
    ));
    btnOpenMask_.reset(new Button(xpm::image(xpm::img_24x24_open),
        [this] () {
            publish_event(this, event_image_frame_open_mask, NULL);
        }
    ));

    for (int i = 0; i < img2img_mode_max; i++) {
        choice_mode_->add(modes_text[i]);
    }

    for (int i = 0; i < brush_size_count; i++) {
        choice_brush_size_->add(brush_captions[i]);
    }

    for (int i = 0; i < inpaint_mode_count; i++) {
        choice_inpaint_mode_->add(inpaint_modes[i]);
    }

    choice_mode_->align(FL_ALIGN_TOP_LEFT);
    choice_brush_size_->align(FL_ALIGN_TOP_LEFT);
    choice_inpaint_mode_->align(FL_ALIGN_TOP_LEFT);
    strength_input_->align(FL_ALIGN_TOP_LEFT);

    choice_mode_->value(0);
    choice_brush_size_->value(5);
    choice_inpaint_mode_->value(0);
    strength_input_->value(75.0);

    choice_mode_->callback(combobox_cb, this);
    choice_brush_size_->callback(combobox_cb, this);
    choice_inpaint_mode_->callback(combobox_cb, this);

    btnNewMask_->tooltip("Create a new mask");
    btnOpenMask_->tooltip("Open a image to use as a mask");

    alignComponents();
    combobox_selected();
}

ImageFrame::~ImageFrame() {
}

img2img_mode_t ImageFrame::get_mode() {
    return mode_;
}

int ImageFrame::get_brush_size() {
    return brush_size_;
}

inpaint_mode_t ImageFrame::get_inpaint_mode() {
    return inpaint_mode_;
}

void ImageFrame::alignComponents() {
    int left = parent_->x();
    int top = parent_->y();
    int w = img_->x() - parent_->x();
    int h = parent_->h();

    choice_mode_->resize(left + 5, top + 25, w - 10, 30);
    choice_inpaint_mode_->resize(left + 5, choice_mode_->h() + choice_mode_->y() + 25, w - 10, 30);
    choice_brush_size_->resize(left + 5, choice_inpaint_mode_->h() + choice_inpaint_mode_->y() + 25, w - 10, 30);
    strength_input_->resize(left + 5, choice_brush_size_->h() + choice_brush_size_->y() + 25, w - 10, 30);

    btnNewMask_->size((w - 15) / 2, 30);
    btnOpenMask_->size(btnNewMask_->w(), btnNewMask_->h());
    btnNewMask_->position(left + 5, strength_input_->y() + strength_input_->h() + 5);
    btnOpenMask_->position(btnNewMask_->x() + btnNewMask_->w() + 5, btnNewMask_->y());
}

void ImageFrame::combobox_cb(Fl_Widget* widget, void *cbdata) {
    static_cast<ImageFrame *>(cbdata)->combobox_cb(widget);
}

void ImageFrame::combobox_cb(Fl_Widget* widget) {
    if (inside_cb_) {
        return;
    }
    inside_cb_ = true;
    combobox_selected();
    publish_event(this, event_image_frame_mode_selected, NULL);
    inside_cb_ = false;
}

void ImageFrame::combobox_selected() {
    if (enabled()) {
        if (parent_->visible_r()) {
            img_->show();
        }
    } else {
        img_->hide();
    }

    if (choice_mode_->value() > 1) {
        btnNewMask_->show();
        btnOpenMask_->show();
        choice_brush_size_->show();
        choice_inpaint_mode_->show();
        strength_input_->hide();
    } else {
        btnNewMask_->hide();
        btnOpenMask_->hide();
        choice_brush_size_->hide();
        choice_inpaint_mode_->hide();
        if (choice_mode_->value() > 0) {
            strength_input_->show();
        }
    }

    mode_ = static_cast<img2img_mode_t>(choice_mode_->value());
    brush_size_ = brushes_sizes[choice_brush_size_->value()];
    inpaint_mode_ = static_cast<inpaint_mode_t>(choice_inpaint_mode_->value());
}

void ImageFrame::enable_mode() {
    if (choice_mode_->value() < 1) {
        choice_mode_->value(1);
    }
    combobox_selected();
}

bool ImageFrame::enabled() {
    return choice_mode_->value() > 0;
}

float ImageFrame::get_strength() {
    float value = 80.0;
    sscanf(strength_input_->value(), "%f", &value);
    if (value < 0.0)
        value = 0;
    if (value > 100.0)
        value = 100.0;
    char buffer[100] = { 0, };
    sprintf(buffer, "%0.1f", value);
    strength_input_->value(buffer);
    return (100.0 - value) / 100.0; 
}

} // namespace dfe
