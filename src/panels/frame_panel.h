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
    image_src_self,
    image_src_results,
    // keep image_src_max at the end
    image_src_max
} image_src_t;

typedef enum {
    image_edit_disabled,
    image_edit_mask,
    image_edit_control_img
} image_edit_t;

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
    FramePanel(image_ptr_t image, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    virtual ~FramePanel();
    void setGridLocation(int index, int variation);
    void enableGrid();
    void enableCache();
    void editMask();
    void editControlImage();
    void setBrushSize(int value);
    void disableEditor();
    void addButton(int id, float xcoord, float ycoord, dexpert::xpm::xpm_t image, frame_btn_cb_t cb);

    size_t gridIndex();
    size_t gridVariation();
    void setImageSource(image_src_t src_type);
    void setImage(image_ptr_t image);
    void clearImage();
    void setMask(image_ptr_t image);
    void clearMask();
    void setControlImg(image_ptr_t image);
    void clearControlImg();

    void setMaskDrawing(bool enabled);
    void setImageDrawing(bool enabled);
    bool getMaskDrawing();
    bool getImageDrawing();

    void redrawIfModified();

    image_ptr_t getImage();
    image_ptr_t getMask();
    image_ptr_t getControlImage();
protected:
    void get_buffer(const unsigned char **buffer, uint32_t *w, uint32_t *h, int *format, int buffer_type) override;
    RawImage *getDrawingImage(int buffer_type);
    void draw_next() override;
    void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) override;
    void mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y) override;
    void draw_mask();

private:
    void drawCircle(int x, int y, bool clear, bool onscreen);
    void update_cache(const unsigned char **buffer, uint32_t *w, uint32_t *h, int channels, size_t version);
    void get_button_coords(frame_button_t *b, float *x, float *y, int *w, int *h);
    frame_button_t *get_button_mouse(int x, int y);
    frame_button_t *get_button_near_mouse(int x, int y);

private:
    bool grid_enabled_ = false;
    bool cache_enabled_ = false;
    bool mask_drawing_ = false;
    bool image_drawing_ = true;
    bool should_draw_brush_ = false;
    bool brush_pos_modified_ = false;
    float brush_pos_x = 0;
    float brush_pos_y = 0;
    uint8_t brush_size_ = 0;
    size_t image_version_ = 0;
    size_t cache_version_ = 0;
    int cache_channels_ = 1;
    size_t cache_w_ = 0;
    size_t cache_h_ = 0;
    size_t cache_bytes_ = 0;
    unsigned char *cache_buffer_ = NULL;

    size_t variation_ = 0;
    size_t index_ = 0;
    image_src_t src_type_ = image_src_self;
    image_edit_t editor_mode_ = image_edit_disabled;

private:
    std::string current_open_dir_;
    std::vector<frame_button_t> buttons_;
    image_ptr_t image_;
    image_ptr_t mask_;
    image_ptr_t control_img_;
};
    
}  // namespace dexpert


#endif  // SRC_PANELS_FRAME_PANEL_H_