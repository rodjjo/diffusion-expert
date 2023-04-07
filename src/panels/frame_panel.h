/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_FRAME_PANEL_H_
#define SRC_PANELS_FRAME_PANEL_H_

#include <string>
#include "src/panels/opengl_panel.h"
#include "src/python/raw_image.h"

namespace dexpert
{

typedef enum {
    image_src_input,
    image_src_results,
    image_src_input_mask,
    image_src_input_scribble,
    image_src_input_pose,
    image_src_controlnet1,
    image_src_controlnet2,
    image_src_controlnet3,
    image_src_controlnet4,
    // keep image_src_max at the end
    image_src_max
} image_src_t;

class FramePanel : public OpenGlPanel {
public:
    FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~FramePanel();
    void setGridLocation(int col, int row);
    void enableGrid();
    void enableCache();

    size_t gridRow();
    size_t gridCol();
    void setImageSource(image_src_t src_type);

protected:
    void get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) override;

private:
    void update_cache(const unsigned char **buffer, uint32_t *w, uint32_t *h, int format, size_t version);

private:
    bool grid_enabled_ = false;
    bool cache_enabled_ = false;
    size_t cache_version_ = 0;
    int cache_type_ = 0;
    size_t cache_w_ = 0;
    size_t cache_h_ = 0;
    size_t cache_bytes_ = 0;
    unsigned char *cache_buffer_ = NULL;

    size_t col_ = 0;
    size_t row_ = 0;
    image_src_t src_type_ = image_src_input;
private:
    std::string current_open_dir_;
    std::shared_ptr<dexpert::py::RawImage> image_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_FRAME_PANEL_H_