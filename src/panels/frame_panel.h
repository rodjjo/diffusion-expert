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

class FramePanel : public OpenGlPanel {
public:
    FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~FramePanel();

protected:
    void get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) override;

private:
    std::string current_open_dir_;
    std::shared_ptr<dexpert::py::RawImage> image_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_FRAME_PANEL_H_