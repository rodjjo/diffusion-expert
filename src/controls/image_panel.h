#ifndef SRC_CONTROLS_IMAGE_PANEL_H
#define SRC_CONTROLS_IMAGE_PANEL_H

#include <functional>

#include "src/panels/opengl_panel.h"
#include "src/python/raw_image.h"

typedef enum {
  image_type_image,         // the final image
  image_type_paste,         // a image that floats before being pasted in the image
  image_type_controlnet,    // pre-processed
  image_type_mask,  
  // keep layer count at the end
  image_type_count
} image_type_t;

typedef enum {
    image_tool_none,
    image_tool_drag,
    image_tool_zoom,
    image_tool_select,
    image_tool_brush,
    // keep image_tool_count at the end
    image_tool_count
} image_tool_t;

typedef struct {
    int x = 0;
    int y = 0;
} coordinate_t;


typedef struct {
    float x = 0.0;
    float y = 0.0;
} fcoordinate_t;

namespace dexpert
{

    typedef std::function<void()> callback_t;

    class ImagePanel : public Fl_Gl_Window
    {
    public:
        ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, callback_t on_change);
        virtual ~ImagePanel();
        void setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setLayerVisible(image_type_t layer, bool visible);
        bool getLayerVisible(image_type_t layer);
        void setLayerEditable(image_type_t layer, bool visible);
        void setActiveLayer(image_type_t layer);
        image_type_t getActiveLayer();
        void setLayerImage(image_type_t layer, image_ptr_t image);
        RawImage* getLayerImage(image_type_t layer);
        image_ptr_t getSelectedImage(image_type_t layer);
        void setPasteImageAtSelection(image_type_t layer, RawImage *img);
        void setZoomLevel(float level);
        float getZoomLevel();
        void setScroll(int x, int y);
        int getScrollX();
        int getScrollY();
        void zoomToFit(float &zoom, int &x, int &y);
        void open(image_type_t layer);
        void save(image_type_t layer);
        void setTool(image_tool_t value);
        image_tool_t getTool();
        static const char* getToolLabel(image_tool_t value);
        void resizeCanvas(uint32_t w, uint32_t h);
        void resizeImages(uint32_t w, uint32_t h);
        void resizeLeft(int value);
        void resizeRight(int value);
        void resizeBottom(int value);
        void resizeTop(int value);
        void setBrushSize(uint8_t size);
        uint8_t getBrushSize();
        coordinate_t getReferenceSize();
        bool hasReference();
        void clearPasteImage();
        void pasteImage();
        bool hasSelection();
        void getSelection(int *x, int *y, int *x2, int *y2);
        void resize(int x, int y, int w, int h) override;
        void getMouseXY(int *x, int *y);
        void newImage(int w, int h);
        void noSelection();
        void upScale(float scale);
        void restoreSelectionFace();
        void cropToSelection();
        void resizeSelection(int w, int h);
    protected:
        int handle(int event) override;
        void draw() override;
        void draw_overlay() override;
        void draw_buffer(RawImage *img);

    protected:
        virtual void mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y, int from_x, int from_y);
        virtual void mouse_down(bool left_button, bool right_button, int down_x, int down_y);
        virtual void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y);
        virtual void mouse_cancel();
        virtual void draw_next();

        bool isSelecting();
        bool isDragging();
        bool isPainting();

    private:
        static void imageRefresh(void *cbdata);
        void imageRefresh();
        void draw_tool();

        RawImage* get_cached_image(int layer);
        RawImage* get_reference_image();
        void fix_scroll(int *xmove, int *ymove);
        void invalidate_caches();

        void scrollAgain();
        fcoordinate_t getDrawingCoord();
        coordinate_t getDrawingCoordScreen();

        void convertToImageCoords(int *x, int *y);
        void convertToScreenCoords(int *x, int *y);
        
    private:
        callback_t on_change_;
        image_tool_t tool_ = image_tool_none;
        bool mouse_changed_ = false;
        int scroll_x_ = 0;
        int scroll_y_ = 0;
        int scroll_px_ = 0;
        int scroll_py_ = 0;
        uint8_t brush_size_ = 16;
        float zoom_ = 1.0f;
        coordinate_t selection_start_ = {0,};
        coordinate_t selection_end_ = {0,};
        uint8_t background_color_[4] = {0, 0, 0, 255};
        image_type_t active_layer_ = image_type_count;
        size_t cache_versions_[image_type_count] = {0,};
        bool valid_caches_[image_type_count] = {0,};
        image_ptr_t caches_[image_type_count];
        coordinate_t paste_coords_;  // positionate the image in relation the image zero
        coordinate_t image_sizes_[image_type_count] = {0,}; // fake the image size, if different of zero
        image_ptr_t images_[image_type_count];
        bool image_visible_[image_type_count] = {0,};
        bool image_editable_[image_type_count] = {0,};

    private:
        bool mouse_down_control_ = false;
        bool mouse_down_shift_ = false;
        bool mouse_down_alt_ = false;
        bool should_redraw_ = false;
        bool mouse_down_left_ = false;
        bool mouse_down_right_ = false;
        int mouse_down_x_ = 0;
        int mouse_down_y_ = 0;
        int move_last_x_ = 0;
        int move_last_y_ = 0;
        int current_x_ = 0;
        int current_y_ = 0;
        viewport_t vp_;

    };
} // namespace dexpert

#endif // SRC_CONTROLS_IMAGE_PANEL_H
