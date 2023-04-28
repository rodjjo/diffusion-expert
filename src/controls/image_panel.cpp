#include <stdio.h>

#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/dialogs/utils.h"
#include "src/controls/image_panel.h"



namespace dexpert
{
    namespace {
        const int gl_format[dexpert::py::img_format_count] = {
            GL_LUMINANCE,
            GL_RGB,
            GL_RGBA
        };
        const char *tool_labels[image_tool_count] = {
            "None",
            "Drag",
            "Zoom",
            "Select",
            "Brush"
        };
    }

    static inline int int_abs(int v) {
        if (v < 0) 
            v = -v;
        return v;
    }

    ImagePanel::ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h) : Fl_Gl_Window(x, y, w, h)
    {
        vp_[0] = 0;
        vp_[1] = 0;
        vp_[2] = this->w();
        vp_[3] = this->h();
        valid(0);
        Fl::add_timeout(0.01, ImagePanel::imageRefresh, this);
    }

    ImagePanel::~ImagePanel()
    {
        Fl::remove_timeout(ImagePanel::imageRefresh, this);
    }

    void ImagePanel::imageRefresh(void *cbdata) {
        ((ImagePanel *) cbdata)->imageRefresh();
        Fl::repeat_timeout(0.10, ImagePanel::imageRefresh, cbdata); // retrigger timeout
    }

    void ImagePanel::imageRefresh() {
        if (should_redraw_) {
            should_redraw_ = false;
            redraw();
        }
    }

    void ImagePanel::setLayerVisible(image_type_t layer, bool visible)
    {
        image_visible_[layer] = visible;
    }

    bool ImagePanel::getLayerVisible(image_type_t layer)
    {
        return image_visible_[layer];
    }

    void ImagePanel::setLayerEditable(image_type_t layer, bool visible)
    {
        image_editable_[layer] = visible;
    }

    void ImagePanel::setActiveLayer(image_type_t layer)
    {
        active_layer_ = layer;
    }

    image_type_t ImagePanel::getActiveLayer()
    {
        return active_layer_;
    }

    void ImagePanel::setLayerImage(image_type_t layer, image_ptr_t image)
    {
        images_[layer] = image;
    }

    RawImage *ImagePanel::getLayerImage(image_type_t layer)
    {
        return images_[layer].get();
    }

    void ImagePanel::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        background_color_[0] = r;
        background_color_[1] = g;
        background_color_[2] = b;
        background_color_[3] = a;
    }

    void ImagePanel::setZoomLevel(float level)
    {
        if (level < 0.05)
            level = 0.05;
        else if (level > 3.0)
            level = 3.0;
        zoom_ = level;
        setScroll(scroll_x_, scroll_y_);
    }

    float ImagePanel::getZoomLevel()
    {
        return zoom_;
    }
    
    void ImagePanel::invalidate_caches() {
        for (uint8_t i = 0; i < image_type_count; ++i) {
            valid_caches_[i] = false;
        }
    }

    void ImagePanel::mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y, int from_x, int from_y){
        current_x_ = move_x;
        current_y_ = move_y;
        if (tool_ == image_tool_brush) {
            should_redraw_ = true;
        }
        auto ref = get_reference_image();
        if (!ref) {
            return;
        }
        if (tool_ == image_tool_drag && left_button) {
            int drag_x = (down_x - move_x) / zoom_;
            int drag_y = (down_y - move_y) / zoom_;
            setScroll(scroll_px_ + drag_x, scroll_py_ + drag_y);
        } else if(tool_ == image_tool_select && left_button) {
            selection_start_.x = down_x;
            selection_start_.y = down_y;
            selection_end_.x = move_x;
            selection_end_.y = move_y;
            should_redraw_ = true;
        }

    };

    void ImagePanel::mouse_down(bool left_button, bool right_button, int down_x, int down_y){
        if (tool_ == image_tool_select && left_button) {
            selection_start_.x = down_x;
            selection_start_.y = down_y;
            selection_end_.x = down_x;
            selection_end_.y = down_y;
            should_redraw_ = true;
        }
    };

    void ImagePanel::mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y){
        if (tool_ == image_tool_select && left_button) {
            selection_start_.x = down_x;
            selection_start_.y = down_y;
            selection_end_.x = up_x;
            selection_end_.y = up_y;
            if (selection_start_.x > selection_end_.x) {
                int tmp = selection_start_.x;
                selection_start_.x = selection_end_.x;
                selection_end_.x = tmp;
            }
            if (selection_start_.y > selection_end_.y) {
                int tmp = selection_start_.y;
                selection_start_.y = selection_end_.y;
                selection_end_.y = tmp;
            }
            should_redraw_ = true;
        }
    };

    void ImagePanel::mouse_cancel() {

    };

    void ImagePanel::draw_next() {

    };

    void ImagePanel::open(image_type_t layer) {
        auto img = open_image_from_dialog();
        if (img) {
            images_[layer] = img;
            if (visible_r()) {
                redraw();
            }
        }
    }

    int ImagePanel::handle(int event)
    {
        switch (event)
        {
        case FL_KEYUP:
        case FL_KEYDOWN:
        {
            return Fl_Gl_Window::handle(event);
        }
        break;

        case FL_MOUSEWHEEL: {
            float z = Fl::event_dy() > 0 ? -0.15 : 0.15;
            setZoomLevel(getZoomLevel() + z);
        }
        break;

        case FL_MOVE:
        case FL_DRAG:
        {
            if (!mouse_down_left_ && !mouse_down_right_)
            {
                mouse_down_x_ = Fl::event_x();
                mouse_down_y_ = Fl::event_y();
            }
            mouse_move(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_, Fl::event_x(), Fl::event_y(), move_last_x_, move_last_y_);
            move_last_x_ = Fl::event_x();
            move_last_y_ = Fl::event_y();
        }
        break;

        case FL_PUSH:
        {
            bool mouse_down_left = Fl::event_button() == FL_LEFT_MOUSE;
            bool mouse_down_right = !mouse_down_left && Fl::event_button() == FL_RIGHT_MOUSE;

            if (mouse_down_left || mouse_down_right)
            {
                mouse_down_x_ = Fl::event_x();
                mouse_down_y_ = Fl::event_y();
                scroll_px_ = scroll_x_;
                scroll_py_ = scroll_y_;
                move_last_x_ = mouse_down_x_;
                move_last_y_ = mouse_down_y_;
                mouse_down_left_ = mouse_down_left;
                mouse_down_right_ = mouse_down_right;
                mouse_down(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_);
            }
            else
            {
                return Fl_Gl_Window::handle(event);
            }
        }
        break;

        case FL_RELEASE:
        {
            if (Fl::event_button() != FL_LEFT_MOUSE && Fl::event_button() != FL_RIGHT_MOUSE)
            {
                return Fl_Gl_Window::handle(event);
            }
            if (mouse_down_left_ || mouse_down_right_)
            {
                mouse_up(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_, Fl::event_x(), Fl::event_y());
                mouse_down_left_ = false;
                mouse_down_right_ = false;
            }
        }
        break;

        default:
            return Fl_Gl_Window::handle(event);
        }

        return 1;
    }

    void ImagePanel::resize(int x, int y, int w, int h) {
        Fl_Gl_Window::resize(x, y, w, h);
        setScroll(scroll_x_, scroll_x_);
    }

    RawImage* ImagePanel::get_cached_image(int layer) {
        /*
            Dim the image to fit the window. Don't draw a huge image in a smaller area...
        */
        RawImage *original = images_[layer].get();
        if (original == NULL) {
            return NULL;
        }
        int w = this->w();
        int h = this->h();
        int remainder = w % 4;

        if (remainder) {
             float yx_relation = h/(float)w;
            w += remainder;    
            h += remainder * yx_relation; // keep it proportional
        }

        image_ptr_t &cache = caches_[layer];
        if (!cache.get() || 
            !valid_caches_[layer] ||
            cache_versions_[layer] != original->getVersion() ||
            cache->w() != w ||  
            cache->h() != h
        )
        {
            valid_caches_[layer] = true;
            cache_versions_[layer] = original->getVersion();
            if (!cache.get() || cache->w() != w || cache->h() != h || cache->format() != original->format()) {
                cache.reset(new RawImage(NULL, w, h, original->format(), false));
            }
            int xmove, ymove;
            fix_scroll(&xmove, &ymove, image_coords_[layer].x, image_coords_[layer].y);
            
            cache->pasteFrom(xmove, ymove, zoom_, original);
        }

        return cache.get();
    }

    void ImagePanel::draw()
    {
        if (!valid())
        {
            valid(1);
            glLoadIdentity();
            glViewport(0, 0, this->w(), this->h());
        }

        vp_.update(); // update the opengl view port

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RawImage *img; 
        for (int i = 0; i < image_type_count; ++i) {
            img = get_cached_image(i);
            if (img) {
                draw_buffer(img);
            }
        }

        draw_tool();
    }
    bool ImagePanel::hasSelection() {
        return selection_start_.x != selection_end_.x && selection_start_.y != selection_end_.y;
    }

    void ImagePanel::getSelection(int *x, int *y, int *x2, int *y2) {
        *x = selection_start_.x;
        *y = selection_start_.y;
        *x2 = selection_end_.x;
        *y2 = selection_end_.y;
    }

    void ImagePanel::draw_tool() {
        float sx = 2.0 / this->w();
        float sy = 2.0 / this->h();
        if (tool_ == image_tool_brush) {
            glBegin(GL_LINE_LOOP);
            float theta;
            float x;
            float y;
            float wx;
            float wy;
            float cx = current_x_;
            float cy = current_y_;
            bool black = true;
            for (int ii = 0; ii < 36; ++ii)   {
                if (black) {
                    black = false;
                    glColor3f(0, 0, 0);
                } else {
                    black = true;
                    glColor3f(1, 1, 1);
                }
                theta = (2.0f * 3.1415926f) * float(ii) / float(36);
                x = (brush_size_ * zoom_) * cosf(theta);
                y = (brush_size_ * zoom_) * sinf(theta);
                wx = ((x + cx) * sx) - 1;
                wy = 1 - ((y + cy) * sy);
                glVertex2f(wx, wy);
            }
            glEnd();
        }

        if (hasSelection()) {
            glBegin(GL_LINE_LOOP);
            glColor3f(0, 0, 0);
            glVertex2f(selection_start_.x * sx - 1.0, 1.0 - selection_start_.y * sy);
            glColor3f(1, 1, 1);
            glVertex2f(selection_end_.x * sx - 1.0, 1.0 - selection_start_.y * sy);
            glColor3f(0, 0, 0);
            glVertex2f(selection_end_.x * sx - 1.0, 1.0 - selection_end_.y * sy);
            glColor3f(1, 1, 1);
            glVertex2f(selection_start_.x * sx - 1.0, 1.0 - selection_end_.y * sy);
            glColor3f(0, 0, 0);
            glVertex2f(selection_start_.x * sx - 1.0, 1.0 - selection_start_.y * sy);
            glEnd();
        }
    }

    void ImagePanel::draw_buffer(RawImage *img) {
        auto ref = get_reference_image();
        if (!img || !ref) {
            return;
        }

        int refw = ref->w() * zoom_;
        int refh = ref->h() * zoom_;
        float dx = -1;
        float dy = 1;
        if (refw < this->w()) {
            dx = -(((2.0 / this->w()) * refw) / 2.0);
        }
        if (refh < this->h()) {
            dy = (((2.0 / this->h()) * refh) / 2.0);
        }
        glRasterPos2f(dx, dy);
        glPixelZoom(1, -1);

        if (img->w() % 4 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        else
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glDrawPixels(img->w(), img->h(), gl_format[img->format()], GL_UNSIGNED_BYTE, img->buffer());

        glRasterPos2f(0.0f, 0.0f);
        glPixelZoom(1.0f, 1.0f);
    }


    void ImagePanel::draw_overlay()
    {
        // Do Nothing
    }

    RawImage* ImagePanel::get_reference_image() {
        if (images_[image_type_image]) {
            return images_[image_type_image].get();
        }
        if (images_[image_type_controlnet]) {
            images_[image_type_controlnet].get();
        }
        return images_[image_type_mask].get();
    }

    void ImagePanel::fix_scroll(int *xmove, int *ymove, int px, int py) {
        RawImage *ref = get_reference_image();
        if (!ref) {
            *xmove = 0;
            *ymove = 0;
            return; // we should have a reference 
        }
        // the images scrolls from 0,0
        // we wan't zero to be its half
        int scroll_x = scroll_x_ + (ref->w() / 2);
        int scroll_y = scroll_y_ + (ref->h() / 2);
        // now we are at the center of the image
        // we need to scroll down to up left coords of the screen
        scroll_x -= (w() / zoom_ ) / 2;
        scroll_y -= (h() / zoom_ ) / 2;
        *xmove = (scroll_x - px); 
        *ymove = (scroll_y - py); 
    }

    void ImagePanel::setScroll(int x, int y) {
        RawImage *img = get_reference_image();
        if (!img) {
            return;
        }
        int halfx = img->w() / 2;
        int halfy = img->h() / 2;
        int halfsx = (this->w() / 2) / zoom_;
        int halfsy = (this->h() / 2) / zoom_;
        if (x + halfsx > halfx) {
            x = halfx - halfsx;
        }
        if (y + halfsy > halfy) {
            y = halfy - halfsy;
        }
        if (x < -halfx) {
            x = -halfx;
        }
        if (y < -halfy) {
            y = -halfy;
        }
        scroll_x_ = x;
        scroll_y_ = y;
        invalidate_caches();
        should_redraw_ = true;
    }

    void ImagePanel::zoomToFit(float &zoom, int &x, int &y) {
        RawImage *img = get_reference_image();
        if (!img) {
            zoom = 1.0;
            x = 0;
            y = 0;
            return;
        }
        vp_.update(w(), h());
        zoom = vp_.raster_zoom(img->w(), img->h());
        point_t raster = vp_.raster_coords(img->w(), img->h());
        x = raster.x * (w() / 2.0);
        y = raster.y * (h() / 2.0);
    }

    

    int ImagePanel::getScrollX() {
        return scroll_x_;
    }

    int ImagePanel::getScrollY() {
        return scroll_y_;
    }

    void ImagePanel::setTool(image_tool_t value) {
        if (value != image_tool_count)
            tool_ = value;
    }

    image_tool_t ImagePanel::getTool() {
        return tool_;
    }

    const char* ImagePanel::getToolLabel(image_tool_t value) {
        if (value < image_tool_count) {
            return tool_labels[value];
        }
        return "Invalid Tool";
    }

} // namespace dexpert