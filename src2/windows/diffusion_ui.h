#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>

#include "python/image.h"

#include "messagebus/messagebus.h"
#include "frames/controlnet_frame.h"
#include "frames/prompt_frame.h"
#include "frames/image_frame.h"
#include "frames/results_frame.h"
#include "components/image_panel.h"
#include "components/button.h"

namespace dfe
{

typedef enum {
    page_type_prompt = 0,
    page_type_image,
    page_type_controlnet1,
    page_type_controlnet2,
    page_type_controlnet3,
    page_type_controlnet4,
    page_type_results,
    // keep page_type_count at the end
    page_type_count 
} page_type_t;
    
class DiffusionWindow: public Fl_Double_Window, public SubscriberThis {
public:
    DiffusionWindow();
    DiffusionWindow(ViewSettings *view_settings);
    ~DiffusionWindow();

protected:
    void resize(int x, int y, int w, int h) override;
    void dfe_handle_event(void *sender, event_id_t event, void *data) override;
    int handle(int event) override;

private:
    void after_constructor();
    void alignComponents();
    static void page_cb(Fl_Widget* widget, void *cbdata);
    void page_cb(Fl_Widget* widget);
    void show_current_page();
    void generate();
    image_ptr_t choose_and_open_image(const char * scope);
    void choose_and_save_image(const char * scope, image_ptr_t image);
    const char *get_mode();
    void show_current_result();
private:
    bool selecting_page_ = false;
    ViewSettings *view_settings_;
    Fl_Group *bottom_panel_;
    Fl_Group *right_panel_;
    Fl_Select_Browser *selector_;
    size_t result_index_ = 0;
    std::vector<image_ptr_t> results_;
    std::unique_ptr<ImageFrame> image_frame_;
    std::unique_ptr<PromptFrame> prompt_frame_;
    std::unique_ptr<ResultFrame> result_frame_;
    std::map<page_type_t, std::unique_ptr<ControlnetFrame> > control_frames_;
    std::map<page_type_t, Fl_Group *> pages_;
    std::map<page_type_t, std::string> titles_;
    std::map<page_type_t, ImagePanel *> images_;
    std::unique_ptr<Button> btnGenerate_;
    std::unique_ptr<Button> btnOk_;
    std::unique_ptr<Button> btnCancel_;
};

image_ptr_t generate_image();
image_ptr_t generate_image(ViewSettings* view_settings);

} // namespace dfe
