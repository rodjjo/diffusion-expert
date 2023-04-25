#include <FL/Fl.H>

#include "src/stable_diffusion/state.h"
#include "src/dialogs/common_dialogs.h"
#include "src/dialogs/utils.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"

#include "src/panels/painting_panel.h"

namespace dexpert
{

namespace {
    const char *modes_text[painting_mode_max] = {
        "Disabled",
        "Img2Img",
        "Inpaint masked",
        "Inpaint not masked",
        "Scribble",
        "Canny",
        "Pose",
        "Deepth"
    };

    const char *controlnet_modes[painting_mode_max] = {
        NULL, // disabled
        NULL, // img2img
        NULL, // inpaint
        NULL, // inpaint
        "scribble",
        "canny",
        "pose",
        "deepth"
    };

    const char *brush_captions[brush_size_count] = {
        "Disabled",
        "1 Pixel",
        "2 Pixels",
        "4 Pixels",
        "8 Pixels",
        "16 Pixels",
        "32 Pixels"
    };
    const uint8_t brushes_sizes[brush_size_count] = {
        0, 1, 2, 4, 8, 16, 32
    };
}

PaintingPanel::PaintingPanel(int x, int y, int w, int h,  PromptPanel *prompt, bool only_control_net): 
        Fl_Group(x, y, w, h), 
        prompt_(prompt), 
        only_control_net_(only_control_net) {
    this->begin();
    image_panel_ = new FramePanel(0, 0, 1, 1);
    image_panel_->setImageSource(image_src_self);
    left_bar_ = new Fl_Group(0, 0, 1, 1);
    left_bar_->begin();
    label_image_ = new Fl_Box(0, 0, 1, 1, "Image");
    label_mask_ = new Fl_Box(0, 0, 1, 1, "Mask");
    label_control_ = new Fl_Box(0, 0, 1, 1, "Transformations");
    mode_ = new Fl_Choice(0, 0, 1, 1, "Mode");
    brushes_ = new Fl_Choice(0, 0, 1, 1, "Brush size");
    denoise_ = new Fl_Float_Input(0, 0, 1, 1, "Similarity");
    btnOpen_.reset(new Button(xpm::image(xpm::directory_16x16), [this] {
       openImage(); 
    }));

    btnSave_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        saveImage();
    }));
    
    btnNewMask_.reset(new Button(xpm::image(xpm::mask_16x16), [this] {
        newMask();
    }));

    btnOpenMask_.reset(new Button(xpm::image(xpm::directory_16x16), [this] {
        openMask();
    }));

    btnSaveMask_.reset(new Button(xpm::image(xpm::save_16x16), [this] {
        saveMask();
    }));

    btnScribble_.reset(new Button(xpm::image(xpm::edit_16x16), [this] {
        extractScribble();
    }));

    btnCanny_.reset(new Button(xpm::image(xpm::button_edit), [this] {
        extractCanny();
    }));

    btnPose_.reset(new Button(xpm::image(xpm::marionette_16x16), [this] {
        extractPose();
    }));

    btnDeepth_.reset(new Button(xpm::image(xpm::eye_16x16), [this] {
        extractDeepth();
    }));

    draw_image_check_ = new Fl_Check_Button(0, 0, 1, 1, "Show reference img");

    left_bar_->end();

    this->end();
    
    btnOpen_->tooltip("Open a image");
    btnOpen_->position(1, 1);
    btnOpen_->size(48, 30);

    btnSave_->tooltip("Save the image");
    btnSave_->position(1, 1);
    btnSave_->size(48, 30);

    btnNewMask_->tooltip("New mask");
    btnNewMask_->position(1, 1);
    btnNewMask_->size(48, 30);

    btnOpenMask_->tooltip("Open a mask");
    btnOpenMask_->position(1, 1);
    btnOpenMask_->size(48, 30);

    btnSaveMask_->tooltip("Save the mask");
    btnSaveMask_->position(1, 1);
    btnSaveMask_->size(48, 30);

    btnScribble_->tooltip("Convert the image to a scribble");
    btnScribble_->position(1, 1);
    btnScribble_->size(35, 30);

    btnCanny_->tooltip("Convert the image to canny edges");
    btnCanny_->position(1, 1);
    btnCanny_->size(35, 30);

    btnPose_->tooltip("Convert the image to pose");
    btnPose_->position(1, 1);
    btnPose_->size(35, 30);
    
    btnDeepth_->tooltip("Convert the image to deepth");
    btnDeepth_->position(1, 1);
    btnDeepth_->size(35, 30);

    denoise_->align(FL_ALIGN_TOP_LEFT);
    denoise_->tooltip("Image similarity %");
    denoise_->value("20");

    for (int i = 0; i < painting_mode_max; ++i) {
        if (!only_control_net || i == 0 || i >= painting_scribble) {
            mode_->add(modes_text[i]);    
        }
    }

    for (int i = 0; i < brush_size_count; ++i) {
        brushes_->add(brush_captions[i]);
    }
    
    draw_image_check_->value(1);

    mode_->value(paiting_disabled);
    mode_->align(FL_ALIGN_TOP);
    mode_->callback(modeSelected, this);
    draw_image_check_->callback(modeSelected, this);

    brushes_->align(FL_ALIGN_TOP);
    brushes_->value(5);
    brushes_->callback(brushSelected, this);

    alignComponents();
    enableControls();
    Fl::add_timeout(0.01, PaintingPanel::imageRefresh, this);
    brushSelected();
}

PaintingPanel::~PaintingPanel() {
    Fl::remove_timeout(PaintingPanel::imageRefresh, this);
}

void PaintingPanel::imageRefresh(void *cbdata) {
    ((PaintingPanel*) cbdata)->image_panel_->redrawIfModified();
    Fl::repeat_timeout(0.10, PaintingPanel::imageRefresh, cbdata); // retrigger timeout
}

void PaintingPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

painting_mode_t PaintingPanel::getSelectedMode() {
    if (only_control_net_) {
        if (mode_->value() == 0) 
            return paiting_disabled;
        return (painting_mode_t) (mode_->value() - 1 + painting_scribble);
    }
    return (painting_mode_t) mode_->value();
}

void PaintingPanel::alignComponents() {
    left_bar_->resize(x(), y(), 150, h());
    image_panel_->resize(x() + left_bar_->w() + 1, y(), w() - left_bar_->w() - 5, h() - 5);

    mode_->resize(left_bar_->x() + 1, left_bar_->y() + 24, left_bar_->w() - 2, 30);
    if (only_control_net_) {
        denoise_->hide();
        denoise_->resize(left_bar_->x() + 1, mode_->y(), mode_->w(), mode_->h());
    } else {
        denoise_->resize(left_bar_->x() + 1, mode_->y() + 1 + mode_->h() + 25, 100, 30);
    }
    label_image_->resize(left_bar_->x() + 1, denoise_->y() + 1 + denoise_->h(), left_bar_->w() - 2, 20);
    btnOpen_->position(left_bar_->x() + 1, label_image_->y() + label_image_->h() + 1);
    btnSave_->position(btnOpen_->x() + 2 + btnOpen_->w(), btnOpen_->y());

    label_mask_->resize(left_bar_->x() + 1, btnOpen_->y() + btnOpen_->h() + 3, left_bar_->w() - 2, 20);
    btnNewMask_->position(left_bar_->x() + 1, label_mask_->y() + label_mask_->h() + 2);
    btnOpenMask_->position(btnNewMask_->x() + 1 + btnNewMask_->w(), btnNewMask_->y());
    btnSaveMask_->position(btnOpenMask_->x() + 1 + btnOpenMask_->w(), btnOpenMask_->y());

    brushes_->resize(left_bar_->x() + 1, btnSaveMask_->y() + btnSaveMask_->h() + 23, left_bar_->w() - 2, 30);
    
    label_control_->resize(left_bar_->x(), brushes_->y() + brushes_->h() + 3, left_bar_->w() - 2, 30);
    btnScribble_->position(left_bar_->x(), brushes_->y() + brushes_->h() + 26);
    btnCanny_->position(btnScribble_->x() + btnScribble_->w() + 2, btnScribble_->y());
    btnPose_->position(btnCanny_->x() + btnCanny_->w() + 2, btnCanny_->y());
    btnDeepth_->position(btnPose_->x() + btnPose_->w() + 2, btnPose_->y());

    draw_image_check_->resize(left_bar_->x(), btnDeepth_->y() + btnDeepth_->h() + 3, left_bar_->w() - 2, 20);
}

void PaintingPanel::enableControls() {
    bool inpanting = false;
    bool image2image = false;
    bool controlnet = false;

    if (getSelectedMode() == painting_img2img) {
        image2image = true;
    }

    if (getSelectedMode() == painting_inpaint_masked ||
        getSelectedMode() == painting_inpaint_not_masked) {
        inpanting = true;
    }

    if (getSelectedMode() == painting_canny ||
        getSelectedMode() == painting_scribble ||
        getSelectedMode() == painting_pose ||
        getSelectedMode() == painting_deepth) {
        controlnet = true;     
    }
    
    if (inpanting | image2image | controlnet) {
        btnOpen_->enabled(true);    
        btnSave_->enabled(true);
        denoise_->activate();
    } else {
        btnOpen_->enabled(false);    
        btnSave_->enabled(false);
        denoise_->deactivate();
        this->top_window()->cursor(FL_CURSOR_DEFAULT);
    }

    if (!only_control_net_) {
        if (inpanting | image2image) {
            denoise_->activate();
        } else {
            denoise_->deactivate();
            this->top_window()->cursor(FL_CURSOR_DEFAULT);
        }
    }

    btnNewMask_->enabled(inpanting || controlnet);
    btnOpenMask_->enabled(btnNewMask_->enabled());
    btnSaveMask_->enabled(btnNewMask_->enabled());

    btnScribble_->enabled(controlnet);
    btnCanny_->enabled(controlnet);
    btnPose_->enabled(controlnet);
    btnDeepth_->enabled(controlnet);
    
    if (inpanting || controlnet) {
        brushes_->activate();
        draw_image_check_->activate();
    } else {
        brushes_->deactivate();
        draw_image_check_->deactivate();
    }

    if (inpanting || image2image || controlnet) {
        image_panel_->show();
    } else {
        image_panel_->hide();
    }
}

void PaintingPanel::saveImage() {
    auto img = image_panel_->getImage();
    if (img) {
        save_image_with_dialog(img);
    }
}

void PaintingPanel::saveMask() {
    image_ptr_t img;
    if (getSelectedMode() == painting_inpaint_masked || 
        getSelectedMode() == painting_inpaint_not_masked) {
        img = image_panel_->getMask();
    } else {
        img = image_panel_->getControlImage();
    }
    if (img) {
        save_image_with_dialog(img);
    }
}

void PaintingPanel::openImage() {
    auto img = open_image_from_dialog();
    if (img) {
        image_panel_->setImage(img);
    }
    modeSelected();
    image_panel_->redraw();
}

void PaintingPanel::openMask() {
    if (getSelectedMode() != painting_inpaint_masked &&
        getSelectedMode() != painting_inpaint_not_masked &&
        getSelectedMode() != painting_pose &&
        getSelectedMode() != painting_canny &&
        getSelectedMode() != painting_scribble &&
        getSelectedMode() != painting_deepth
    ) {
        show_error("This mode does not allow masks!");
        return;
    }
    auto img = open_image_from_dialog();
    if (img) {
        if (img->format() != dexpert::py::img_rgba && getSelectedMode() != painting_deepth) {
            show_error("The image does not have alpha channel.");
            return;
        }
        if (getSelectedMode() == painting_inpaint_masked ||
            getSelectedMode() == painting_inpaint_not_masked
        ) {
            image_panel_->setMask(img);
        } else {
            image_panel_->setControlImg(img);
        }
    }
    modeSelected();
    image_panel_->redraw();
}

void PaintingPanel::brushSelected(Fl_Widget *widget, void *cbdata) {
    ((PaintingPanel *) cbdata)->brushSelected();
}

void PaintingPanel::brushSelected() {
    image_panel_->setBrushSize(brushes_sizes[brushes_->value()]);
}

void PaintingPanel::modeSelected(Fl_Widget *widget, void *cbdata) {
    ((PaintingPanel *) cbdata)->modeSelected();
}

void PaintingPanel::modeSelected() {
    switch (getSelectedMode()) {
        case painting_inpaint_masked:
        case painting_inpaint_not_masked:
            image_panel_->setMaskDrawing(true);
            image_panel_->setImageDrawing(draw_image_check_->value() == 1);
            image_panel_->editMask();
        break;
        case painting_pose:
        case painting_canny:
        case painting_scribble:
        case painting_deepth:
            image_panel_->setMaskDrawing(true);
            image_panel_->setImageDrawing(draw_image_check_->value() == 1);
            image_panel_->editControlImage();
        break;
        default:
            image_panel_->setMaskDrawing(false);
            image_panel_->setImageDrawing(true);
            image_panel_->disableEditor();
        break;
    }
    enableControls();
}

void PaintingPanel::newMask() {
    if (getSelectedMode() == painting_inpaint_masked ||
        getSelectedMode() == painting_inpaint_not_masked
    ) {
        bool should_continue = image_panel_->getMask() ? false : true;
        should_continue = (should_continue || ask("Do you want to create a new mask ?"));
        if (should_continue) {
            image_panel_->setMask(dexpert::py::newImage(prompt_->getWidth(), prompt_->getHeight(), true));
            image_panel_->editMask();
            image_panel_->redraw();
        }
    } else if (getSelectedMode() == painting_pose ||
        getSelectedMode() == painting_canny ||
        getSelectedMode() == painting_scribble
    ) {
        bool should_continue = image_panel_->getControlImage() ? false : true;
        should_continue = (should_continue || ask("Do you want to create a blank canvas ?"));
        if (should_continue) {
            image_panel_->setControlImg(dexpert::py::newImage(prompt_->getWidth(), prompt_->getHeight(), true));
            image_panel_->editControlImage();
            image_panel_->redraw();
        }
    } else {
        show_error("This mode does not allow mask");
    }
}

void PaintingPanel::setImage(RawImage *image) {
    if (image) {
        image_panel_->setImage(image->duplicate());
    }
}

bool PaintingPanel::ensureControlPresent() {
     if (image_panel_->getControlImage()) {
        return true;
    }
    show_error("There is no image to proceed");
    return false;
}

bool PaintingPanel::ensureImagePresent() {
    if (image_panel_->getImage()) {
        return true;
    }
    show_error("There is no image to proceed");
    return false;
}

bool PaintingPanel::ensureMaskPresent() {
    if (image_panel_->getMask()) {
        return true;
    }
    show_error("There is no mask to proceed");
    return false;
}

void PaintingPanel::pre_process(const char* method) {
    if (!ensureImagePresent()) {
        return;
    }
    if (!ask("The operation will replace the input image. Do you want to proceed ?"))  {
        return;
    }
    const char *msg = NULL;
    bool success = false;
    dexpert::py::image_ptr_t img;
    dexpert::py::get_py()->execute_callback(
        dexpert::py::pre_process_image(
            method,
            image_panel_->getImage().get(),
            [&msg, &success, &img] (bool suc, const char *message, dexpert::py::image_ptr_t image) {
                msg = message;
                success = suc;
                img = image;
        })
    );
    if (msg) {
        show_error(msg);
    } else {
        if (method == std::string("canny")) {
            mode_->value(painting_canny);
        } else if (method == std::string("pose")) {
            mode_->value(painting_pose);
        } else if (method == std::string("scriblle")) {
            mode_->value(painting_scribble);
        } else if (method == std::string("deepth")) {
            mode_->value(painting_deepth);
        }

        if (method != std::string("deepth")) {
            img = img->removeBackground(getSelectedMode() != painting_pose);
        }

        image_panel_->setControlImg(img);
        image_panel_->editControlImage();
        image_panel_->setMaskDrawing(true);
        image_panel_->setImageDrawing(draw_image_check_->value() == 1);
        image_panel_->redraw();
    }
}

void PaintingPanel::extractCanny() {
    pre_process("canny");
}

void PaintingPanel::extractScribble() {
    pre_process("scribble");
}

void PaintingPanel::extractPose() {
    pre_process("pose");
}

void PaintingPanel::extractDeepth() {
    pre_process("deepth");
}

image_ptr_t PaintingPanel::getImg2ImgImage() {
    switch (getSelectedMode())
    {
        case painting_img2img:
        case painting_inpaint_masked:
        case painting_inpaint_not_masked:
            return image_panel_->getImage();
        break;
    } 

    return image_ptr_t();
}

image_ptr_t PaintingPanel::getImg2ImgMask() {
    switch (getSelectedMode())
    {
        case painting_img2img:
        case painting_inpaint_masked:
        case painting_inpaint_not_masked:
            return image_panel_->getMask();
        break;
        
        default:
            break;
    }
    return image_ptr_t();
}

bool PaintingPanel::ready() {
    if (getSelectedMode() == paiting_disabled) {
        return prompt_->ready(true);
    }
    if (getSelectedMode() == painting_inpaint_masked ||
        getSelectedMode() == painting_inpaint_not_masked) {
        return ensureImagePresent() && ensureMaskPresent();
    }
    if (getSelectedMode() == painting_scribble ||
        getSelectedMode() == painting_canny || 
        getSelectedMode() == painting_pose ||
        getSelectedMode() == painting_deepth
    ) {
        return ensureControlPresent();
    }
    return ensureImagePresent();
}


bool PaintingPanel::should_invert_mask_colors() {
    return getSelectedMode() == painting_inpaint_masked;
}

std::shared_ptr<ControlNet> PaintingPanel::getControlnet() {
    std::shared_ptr<ControlNet> result;
    const char *mode = controlnet_modes[getSelectedMode()];

    if (mode != NULL) {
        if (ensureControlPresent()) {
            auto img = image_panel_->getControlImage();
            if (getSelectedMode() == painting_deepth)
                img = img->duplicate();
            else
                img = img->removeAlpha();
            result.reset(new ControlNet(mode, img));
        }
    }
     
    return result;
}

float PaintingPanel::get_denoise_strength() {
    float value = 80.0;
    sscanf(denoise_->value(), "%f", &value);
    if (value < 0.0)
        value = 0;
    if (value > 100.0)
        value = 100.0;
    char buffer[100] = { 0, };
    sprintf(buffer, "%0.1f", value);
    denoise_->value(buffer);
    return (100.0 - value) / 100.0; 
}
    
} // namespace dexpert
