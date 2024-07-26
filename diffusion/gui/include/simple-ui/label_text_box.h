#pragma once

#include "simple-ui/edit.h"
#include "simple-ui/label.h"

namespace dfe_ui
{

class LabelTextbox : public Component {
   public:
    LabelTextbox(Window * window, int x, int y, int w, int h, const char *label);
    ~LabelTextbox();

};


} // namespace name
