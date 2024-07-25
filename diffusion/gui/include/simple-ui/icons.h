#pragma once

#include <memory>

namespace dfe_ui
{

typedef enum {
    img_24x24_new,
    img_24x24_new_document,
    img_24x24_open,
    img_24x24_folder,
    img_24x24_wallet,
    img_24x24_flash_drive,
    img_24x24_pinion,
    img_24x24_open_layer,
    img_24x24_remove,
    img_24x24_erase,
    img_24x24_close,
    img_24x24_settings,
    img_24x24_bee,
    img_24x24_copy,
    img_24x24_picture,
    img_24x24_exit,
    img_24x24_ok,
    img_24x24_abort,
    img_24x24_magic_wand,
    img_24x24_back,
    img_24x24_forward,
    img_24x24_green_pin,
    img_24x24_heart,
    img_24x24_medium_rating,
    cur_24x24_arrow,
    // keep img_none at the end
    img_none
} icon_type_t;


class IconTextureBase {
    public:
        virtual ~IconTextureBase() {}
        virtual void *sfml_texture() = 0;
        virtual std::pair<int, int> get_coords(icon_type_t xpm_id) = 0;
        virtual std::pair<int, int> get_size(icon_type_t xpm_id) = 0;
};

IconTextureBase* load_icons_texture();
void unload_icons_texture();

} // namespace dfe_ui
