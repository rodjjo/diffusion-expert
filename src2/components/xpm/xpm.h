#pragma once

#include <memory>

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Pixmap.H>


namespace dfe {
namespace xpm {

typedef enum {
    img_24x24_new = 0,
    img_24x24_open,
    img_24x24_open_layer,
    img_24x24_remove,
    img_24x24_erase,
    img_24x24_close,
    img_24x24_settings,
    img_24x24_copy,
    img_24x24_picture,
    img_24x24_exit,
    // keep no_image at the end
    no_image
} xpm_t;

std::shared_ptr<Fl_RGB_Image> image(xpm_t xpm_id,  Fl_Color bg=FL_GRAY);

} // namespace xpm
} // namespace dexpert

