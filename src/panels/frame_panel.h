/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_FRAME_PANEL_H_
#define SRC_PANELS_FRAME_PANEL_H_

#include <vector>
#include <string>
#include <functional>
#include "src/panels/opengl_panel.h"
#include "src/python/raw_image.h"
#include "src/data/xpm.h"


namespace dexpert
{
class FramePanel;

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

typedef std::function<void(FramePanel *frame, int id)> frame_btn_cb_t;

typedef struct {
    int id;
    float x;
    float y;
    frame_btn_cb_t cb;
    std::shared_ptr<Fl_RGB_Image> image;
} frame_button_t;

class FramePanel : public OpenGlPanel {
public:
    FramePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~FramePanel();
    void setGridLocation(int index, int variation);
    void enableGrid();
    void enableCache();
    void addButton(int id, float xcoord, float ycoord, dexpert::xpm::xpm_t image, frame_btn_cb_t cb);

    size_t gridIndex();
    size_t gridVariation();
    void setImageSource(image_src_t src_type);

protected:
    void get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format) override;
    void draw_next() override;
    void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) override;
private:
    void update_cache(const unsigned char **buffer, uint32_t *w, uint32_t *h, int channels, size_t version);
    void get_button_coords(frame_button_t *b, float *x, float *y, int *w, int *h);
    frame_button_t *get_button_mouse(int x, int y);
    frame_button_t *get_button_near_mouse(int x, int y);

private:
    bool grid_enabled_ = false;
    bool cache_enabled_ = false;
    size_t cache_version_ = 0;
    int cache_channels_ = 1;
    size_t cache_w_ = 0;
    size_t cache_h_ = 0;
    size_t cache_bytes_ = 0;
    unsigned char *cache_buffer_ = NULL;

    size_t variation_ = 0;
    size_t index_ = 0;
    image_src_t src_type_ = image_src_input;
private:
    std::string current_open_dir_;
    std::vector<frame_button_t> buttons_;
    std::shared_ptr<dexpert::py::RawImage> image_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_FRAME_PANEL_H_