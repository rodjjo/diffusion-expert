#include "components/xpm/xpm.h"
#include "messagebus/messagebus.h"

#include "frames/results_frame.h"

namespace dfe
{

ResultFrame::ResultFrame(Fl_Group *parent, ImagePanel *img) {
    parent_ = parent;
    img_ = img;
    btn_previous_.reset(new Button(
        xpm::image(xpm::img_24x24_back),
        [this] () {
            publish_event(this, event_generator_previous_image, NULL);
        }
    ));
    btn_next_.reset(new Button(
        xpm::image(xpm::img_24x24_forward),
        [this] () {
            publish_event(this, event_generator_next_image, NULL);
        }
    ));
    btn_accept_.reset(new Button(
        xpm::image(xpm::img_24x24_heart),
        [this] () {
            publish_event(this, event_generator_accept_image, NULL);
        }
    ));
    btn_accept_partially_.reset(new Button(
        xpm::image(xpm::img_24x24_medium_rating),
        [this] () {
            publish_event(this, event_generator_accept_partial_image, NULL);
        }
    ));
    btn_previous_->tooltip("Go to the previous generated image...");
    btn_next_->tooltip("Go to the next generated image...");
    btn_accept_->tooltip("Send to the input image tab");
    btn_accept_partially_->tooltip("Select a region in the currenct image and send to image tab");
}

ResultFrame::~ResultFrame() {
    
}

void ResultFrame::alignComponents() {
    int left = parent_->x();
    int top = parent_->y();
    int w = img_->x() - parent_->x();
    int h = parent_->h();
    btn_previous_->position(left + 5, top + 5);
    btn_previous_->size((w - 15) / 2, 30);
    btn_next_->position(btn_previous_->x() + btn_previous_->w() + 5, btn_previous_->y());
    btn_next_->size(btn_previous_->w(), 30);
    btn_accept_->position(left + 5, btn_previous_->y() + btn_previous_->h() + 5);
    btn_accept_->size(w - 10, 30);
    btn_accept_partially_->position(btn_accept_->x(), btn_accept_->y() + btn_accept_->h() + 5);
    btn_accept_partially_->size(w - 10, 30);
}
    
} // namespace dfe