
#include "misc/utils.h"
#include "misc/gen_parameters.h"
#include "components/xpm/xpm.h"
#include "python/routines.h"

#include "windows/diffusion_ui.h"

namespace dfe
{
    namespace {
        const std::list<event_id_t> diffusion_tool_events = {
            event_generator_next_image,
            event_generator_previous_image,
            event_generator_accept_image,
            event_generator_accept_partial_image
        };

        class NonEditableImagePanel : public ImagePanel {
            public:
                NonEditableImagePanel(
                    uint32_t x, uint32_t y, 
                    uint32_t w, uint32_t h, 
                    const char *unique_title) : ImagePanel(
                        x, y, w, h, unique_title
                    ) {};

                bool enable_selection() override {
                    // do not let the user to select a region
                    return false;
                }

                bool enable_drag() override {
                    // do not let the user to drag the image
                    return false;
                }

                bool enable_resize() override {
                    // do not let the user to change the size of the image
                    return false;
                }
        };
    }

    DiffusionWindow::DiffusionWindow() : Fl_Window(
        Fl::w() / 2 - 860 / 2, Fl::h() / 2 - 640 / 2,
        860, 640, "Image generator"
    ), view_settings_(NULL), SubscriberThis(diffusion_tool_events)  {
        after_constructor();
    }

    DiffusionWindow::DiffusionWindow(ViewSettings *view_settings) : Fl_Window(
        Fl::w() / 2 - 860 / 2, Fl::h() / 2 - 640 / 2,
        860, 640, "Image generator"
    ), view_settings_(view_settings), SubscriberThis(diffusion_tool_events) {
        after_constructor();
    }

    void DiffusionWindow::after_constructor() {
        this->size_range(this->w(), this->h());
        this->begin();
        bottom_panel_ = new Fl_Group(0, 0, 1, 1);
        bottom_panel_->box(FL_DOWN_BOX);
        {
            bottom_panel_->begin();
            btnOk_.reset(new Button(xpm::image(xpm::img_24x24_ok), [this] {
                
            }));

            btnCancel_.reset(new Button(xpm::image(xpm::img_24x24_abort), [this] {
                this->hide();        
            }));

            bottom_panel_->end();
        }
        this->begin();
        btnGenerate_.reset(new Button(xpm::image(xpm::img_24x24_magic_wand), [this] {
            generate();
        }));
        right_panel_ = new Fl_Group(0, 0, 1, 1);
        right_panel_->box(FL_DOWN_BOX);
        { // page selector
            right_panel_->begin();
            const char *page_names[page_type_count] = {
                "Prompts",
                "Base Image",
                "Controlnet 1",
                "Controlnet 2",
                "Controlnet 3",
                "Controlnet 4",
                "Generate"
            };
            selector_ = new Fl_Select_Browser(0, 0, 1, 1);
            for (int i = (page_type_t)0; i < page_type_count; i++) {
                selector_->add(page_names[i]);
            }
            selector_->callback(page_cb, this);
            right_panel_->end();
        }
        page_type_t where;
        
        for (int i = (page_type_t)0; i < page_type_count; i++) {
            where = static_cast<page_type_t>(i);
            images_[where] = NULL;
            char buffer[128];
            sprintf(buffer, "DiffusionWindow_%d", i);
            this->begin();
            pages_[where] = new Fl_Group(0, 0, 1, 1);
            pages_[where]->box(FL_DOWN_BOX);
            pages_[where]->begin();
            if (i != page_type_prompt) {
                titles_[where] = buffer;
                images_[where] = new NonEditableImagePanel(0, 0, 1, 1, titles_[where].c_str());
                if (i == page_type_image) {
                    image_frame_.reset(new ImageFrame(pages_[where],  images_[where]));
                } else if (i == page_type_results) {
                    result_frame_.reset(new ResultFrame(pages_[where],  images_[where]));
                } else {
                    control_frames_[where] = std::unique_ptr<ControlnetFrame>(new ControlnetFrame(pages_[where],  images_[where]));
                }
            } else {
                prompt_frame_.reset(new PromptFrame(pages_[where]));
            }
            pages_[where]->end();
            pages_[where]->hide();
        }
        this->end();

        alignComponents();
        selector_->value(1);
        show_current_page();
        prompt_frame_->refresh_models();
    }

    DiffusionWindow::~DiffusionWindow() {

    }

    void DiffusionWindow::resize(int x, int y, int w, int h) {
        Fl_Window::resize(x, y, w, h);
        alignComponents();
    }

    void DiffusionWindow::alignComponents() {
        bottom_panel_->resize(0, 0, w() - 6, 40);
        bottom_panel_->position(3, h() - bottom_panel_->h() - 3);

        btnOk_->position(this->w() - 215, this->h() - 37);
        btnOk_->size(100, 30);
        btnCancel_->position(btnOk_->x() + btnOk_->w() + 2, btnOk_->y());
        btnCancel_->size(100, 30);
        btnGenerate_->size(100, 30);
        btnGenerate_->position(w() - btnGenerate_->w() - 5, 5);
        right_panel_->resize(0, 0, btnGenerate_->w(), h() - bottom_panel_->h() - btnGenerate_->h() - 20);
        right_panel_->position(w() - right_panel_->w() - 5, btnGenerate_->y() + btnGenerate_->h() + 5);
        selector_->resize(
            right_panel_->x(),
            right_panel_->y(),
            right_panel_->w(),
            right_panel_->h()
        );

        page_type_t where;
        for (int i = (page_type_t)0; i < page_type_count; i++) {
            where = static_cast<page_type_t>(i);
            pages_[where]->resize(5, 5, w() - right_panel_->w() - 17, h() - bottom_panel_->h() - 14);
            pages_[where]->position(7, 7);
            if (images_[where]) {
                images_[where]->resize(5, 5, pages_[where]->w() - 150, pages_[where]->h() - 14);
                images_[where]->position(pages_[where]->w() - images_[where]->w() - 7, pages_[where]->y() + 7);
            }
        }

        prompt_frame_->alignComponents();
        image_frame_->alignComponents();
        result_frame_->alignComponents();
        for (int i = page_type_controlnet1; i < page_type_controlnet4 + 1; i++)  {
            where = static_cast<page_type_t>(i);
            control_frames_[where]->alignComponents();
        }
    }
    
    void DiffusionWindow::page_cb(Fl_Widget* widget, void *cbdata) {
        static_cast<DiffusionWindow*>(cbdata)->page_cb(widget);
    }

    void DiffusionWindow::page_cb(Fl_Widget* widget) {
        if (selecting_page_) {
            return;
        }
        selecting_page_ = true;
        int idx = selector_->value();
        if (idx > 0)  {
            show_current_page();
        } 
        selector_->deselect();
        selector_->select(idx);
        selecting_page_ = false;
    }

    void DiffusionWindow::show_current_page() {
        page_type_t where;
        for (int i = (page_type_t)0; i < page_type_count; i++) {
            where = static_cast<page_type_t>(i);
            pages_[where]->hide();
        }
        int idx = selector_->value();
        if (idx > 0)  {
            where = static_cast<page_type_t>(idx - 1);
            pages_[where]->show();
            if (images_[where] && !images_[where]->shown()) {
                images_[where]->show();
            }
            if (where == page_type_image) {
                btnOk_->show();
            } else {
                btnOk_->hide();
            }
        }
    }

    void DiffusionWindow::dfe_handle_event(void *sender, event_id_t event, void *data) {
        if (sender == result_frame_.get()) {
            switch (event) {
                case event_generator_next_image:

                break;

                case event_generator_previous_image:
                break;

                case event_generator_accept_image:
                break;

                case event_generator_accept_partial_image:
                break;
            };
        }
    }

    void DiffusionWindow::generate() {
        GeneratorParameters params;
        params.positive_prompt = prompt_frame_->positive_prompt();
        params.negative_prompt = prompt_frame_->negative_prompt();
        params.seed = prompt_frame_->get_seed();
        params.steps = prompt_frame_->get_steps();
        params.batch_size = prompt_frame_->get_batch_size();
        params.model = prompt_frame_->get_model();
        params.inpaint_model = prompt_frame_->get_inpaint_model();
        params.cfg = prompt_frame_->get_cfg();
        params.mode = "txt2img";
        params.scheduler_name = "EulerAncestralDiscreteScheduler";
        params.width = prompt_frame_->get_width();
        params.height = prompt_frame_->get_height();
        params.use_lcm_lora = prompt_frame_->use_lcm_lora();
        params.use_tiny_vae = prompt_frame_->use_tiny_vae();
        auto result = py::generate_image(params.toDict());
        if (!result.empty()) {
            images_[page_type_results]->view_settings()->set_image(*result.begin());
        }
    }

    image_ptr_t generate_image_(ViewSettings* view_settings) {
        image_ptr_t r;
        DiffusionWindow *window = view_settings ? new DiffusionWindow(view_settings) : new DiffusionWindow();
        window->show();
        while (true) {
            if (!window->visible_r()) {
                break;
            }
            Fl::wait();
        }
        Fl::delete_widget(window);
        Fl::do_widget_deletion();
        return r;
    }

    image_ptr_t generate_image() {
        return generate_image_(NULL);
    }

    image_ptr_t generate_image(ViewSettings* view_settings) {
        return generate_image_(view_settings);
    }

} // namespace dfe
