#include "src/stable_diffusion/controlnet.h"

namespace dexpert
{

ControlNet::ControlNet(const char *mode, image_ptr_t image, float strength) :
            mode_(mode), strength_(strength), image_(image) {
    if (strength_ < 0) {
        strength_ = 0;
    }
    if (strength_ > 2.0) {
        strength_ = 2.0;
    }
}

ControlNet::~ControlNet() {

}

float ControlNet::getStrenght() {
    return strength_;
}

const char *ControlNet::getMode() {
    return mode_.c_str();
}
RawImage *ControlNet::getImage() {
    return image_.get();
}

} // namespace dexpert
