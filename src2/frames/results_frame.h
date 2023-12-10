#pragma once

#include <memory>

#include <FL/Fl_Group.H>
#include "components/image_panel.h"
#include "components/button.h"


namespace dfe
{

class ResultFrame {
public:
    ResultFrame(Fl_Group *parent, ImagePanel *img);
    ~ResultFrame();
    void alignComponents();
private:
    Fl_Group *parent_;
    ImagePanel *img_;
    std::unique_ptr<Button> btn_previous_;
    std::unique_ptr<Button> btn_next_;
    std::unique_ptr<Button> btn_accept_;
    std::unique_ptr<Button> btn_accept_partially_;
};

    
} // namespace dfe
