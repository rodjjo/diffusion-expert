 #include "src/windows/restoration_params.h"

namespace dexpert
{

RestorationParams::RestorationParams() : Fl_Window(480, 200, "Upscaler parameters") {
    this->size_range(480, 200);
    this->set_modal();
 }

RestorationParams::~RestorationParams() {

}

    
} // namespace dexpert
