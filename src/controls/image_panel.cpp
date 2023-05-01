#include <stdio.h>

#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/dialogs/utils.h"
#include "src/dialogs/common_dialogs.h"
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
            "Drag Floating Image"
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

    ImagePanel::ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, callback_t on_change) : Fl_Gl_Window(x, y, w, h)
    {
        on_change_ = on_change;
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
            mouse_changed_ = false;
            if (on_change_) {
                on_change_();
            }
            redraw();
        } else if (mouse_changed_) {
            mouse_changed_ = false;
            if (on_change_) {
                on_change_();
            }
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

    void restrictSelection(coordinate_t &coord, RawImage& img) {
        if (coord.x < 0) coord.x = 0;
        if (coord.y < 0) coord.y = 0;
        if (coord.x >= img.w()) coord.x = img.w() - 1;
        if (coord.y >= img.h()) coord.y = img.h() - 1;
    }

    image_ptr_t ImagePanel::getSelectedImage(image_type_t layer) {
        if (hasSelection() && images_[layer].get() != NULL) {
            RawImage &img = *images_[layer].get();
            coordinate_t s1 = selection_start_, s2 = selection_end_;
            restrictSelection(s1, img);
            restrictSelection(s2, img);
            int w = s2.x - s1.x;
            int h = s2.y - s1.y;
            if (w > 0 && h > 0) {
                return img.getCrop(s1.x, s1.y, w, h);
            }
        }
        return image_ptr_t();
    }

    void ImagePanel::setPasteImageAtSelection(image_type_t layer, RawImage *img) {
         if (hasSelection() && img != NULL && images_[layer].get() != NULL) {
            RawImage &ref = *images_[layer].get();
            coordinate_t s1 = selection_start_, s2 = selection_end_;
            restrictSelection(s1, ref);
            restrictSelection(s2, ref);
            int w = s2.x - s1.x;
            int h = s2.y - s1.y;
            if (w > 0 && h > 0) {
                images_[image_type_paste] = img->resizeCanvas(w, h);
                paste_coords_.x = s1.x;
                paste_coords_.y = s1.y;
                setScroll(scroll_x_, scroll_y_);
            }
         }
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
        scrollAgain();
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

    bool ImagePanel::isSelecting() {
        if (images_[image_type_paste]) {
            selection_start_ = paste_coords_;
            selection_end_ = paste_coords_;
            selection_end_.x += images_[image_type_paste]->w();
            selection_end_.y += images_[image_type_paste]->h();
            return false;
        }
        return tool_ == image_tool_select && mouse_down_left_;
    }

    bool ImagePanel::isDragging() {
         if (mouse_down_left_ && mouse_down_control_) {
            return true;
         }
         return tool_ == image_tool_drag && mouse_down_left_;
    }

    bool ImagePanel::isPainting() {
        return (tool_ == image_tool_brush) && mouse_down_left_ && !isDragging();
    }

    void ImagePanel::getMouseXY(int *x, int *y) {
        *x = current_x_;
        *y = current_y_;
        convertToImageCoords(x, y);
    }


    void ImagePanel::mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y, int from_x, int from_y){
        current_x_ = move_x;
        current_y_ = move_y;
        mouse_changed_ = true;
        if (tool_ == image_tool_brush) {
            should_redraw_ = true;
        }
        auto ref = get_reference_image();
        if (!ref) {
            return;
        }

        if (isDragging()) {
            int drag_x = (down_x - move_x) / zoom_;
            int drag_y = (down_y - move_y) / zoom_;
            setScroll(scroll_px_ + drag_x, scroll_py_ + drag_y);
        } else if(isSelecting() && left_button) {
            selection_start_.x = down_x;
            selection_start_.y = down_y;
            selection_end_.x = move_x;
            selection_end_.y = move_y;
            should_redraw_ = true;
        }
    };

    void ImagePanel::mouse_down(bool left_button, bool right_button, int down_x, int down_y){
        if (isDragging()) {
            return;
        }

        if (isSelecting() && left_button) {
            selection_start_.x = down_x;
            selection_start_.y = down_y;
            selection_end_.x = down_x;
            selection_end_.y = down_y;
            should_redraw_ = true;
        }
    };

    void ImagePanel::mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y){
        if (isDragging()) {
            return;
        }

        if (isSelecting() && left_button) {
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
            convertToImageCoords(&selection_start_.x, &selection_start_.y);
            convertToImageCoords(&selection_end_.x, &selection_end_.y);
            should_redraw_ = true;
        }
    };

    void ImagePanel::mouse_cancel() {

    };

    void ImagePanel::draw_next() {

    };

    void ImagePanel::open(image_type_t layer) {
        if (images_[image_type_paste]) {
            show_error("Can not open with floating image being processed");
            return;  // wait the user to decide what he's going to do with the floating image
        }

        auto img = open_image_from_dialog();
        if (img) {
            images_[layer] = img;
            if (visible_r()) {
                scrollAgain();
            }
        }
    }

    void ImagePanel::clearPasteImage() {
        images_[image_type_paste].reset();
        paste_coords_.x = 0;
        paste_coords_.y = 0;
        scrollAgain();
    }

    void ImagePanel::pasteImage() {
        if (!images_[image_type_paste] || !images_[image_type_image]) {
            return;
        }
        images_[image_type_image]->pasteAt(paste_coords_.x, paste_coords_.y, images_[image_type_paste].get());
        images_[image_type_paste].reset();
        paste_coords_.x = 0;
        paste_coords_.y = 0;
        selection_start_ = selection_end_;
        scrollAgain();
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
            if (get_reference_image()) {
                float z = Fl::event_dy() > 0 ? -0.05 : 0.05;
                setZoomLevel(getZoomLevel() + z);
            }
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
            mouse_down_control_ = false;
            mouse_down_shift_ = false;
            mouse_down_alt_ = false;
            if (mouse_down_left || mouse_down_right)
            {
                mouse_down_control_ = Fl::event_command() != 0;
                mouse_down_shift_ = Fl::event_shift() != 0;
                mouse_down_alt_ = Fl::event_alt() != 0;;
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
                mouse_down_control_ = false;
                mouse_down_shift_ = false;
                mouse_down_alt_ = false;
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
        scrollAgain();
    }

    RawImage* ImagePanel::get_cached_image(int layer) {
        /*
            Dim the image to fit the window. Don't draw a huge image in a smaller area...
        */
        RawImage *original = images_[layer].get();
        if (original == NULL || layer == image_type_paste) {
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
            if (!cache.get() || cache->w() != w || cache->h() != h /*|| cache->format() != original->format()*/) {
                cache.reset(new RawImage(NULL, w, h, dexpert::py::img_rgba, false));
            }
            int xmove, ymove;
            fix_scroll(&xmove, &ymove);
            cache->pasteFrom(xmove, ymove, zoom_, original);
            if (layer == image_type_image && images_[image_type_paste].get() != NULL) {
                RawImage *img = images_[image_type_paste].get();
                coordinate_t s1 = paste_coords_;
                s1.x *=  zoom_;
                s1.y *=  zoom_;
                s1.x -= xmove * zoom_;
                s1.y -= ymove * zoom_;
                cache->pasteAt(s1.x , s1.y, img->w() * zoom_, img->h() * zoom_, img);
            }
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
        if (isSelecting()) {
            if (*x > *x2) {
                *x2 = selection_start_.x;
                *x  = selection_end_.x;
            }
            if (*y > *y2) {
                *y2 = selection_start_.y;
                *y  = selection_end_.y;
            }
            convertToImageCoords(x, y);
            convertToImageCoords(x2, y2);
        }
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
            coordinate_t s1 = selection_start_;
            coordinate_t s2 = selection_end_;
            if (!isSelecting()) {
                convertToScreenCoords(&s1.x, &s1.y);
                convertToScreenCoords(&s2.x, &s2.y);
            }
            glBegin(GL_LINE_LOOP);
            glColor3f(0, 0, 0);
            glVertex2f(s1.x * sx - 1.0, 1.0 - s1.y * sy);
            glColor3f(1, 1, 1);
            glVertex2f(s2.x * sx - 1.0, 1.0 - s1.y * sy);
            glColor3f(0, 0, 0);
            glVertex2f(s2.x * sx - 1.0, 1.0 - s2.y * sy);
            glColor3f(1, 1, 1);
            glVertex2f(s1.x * sx - 1.0, 1.0 - s2.y * sy);
            glColor3f(0, 0, 0);
            glVertex2f(s1.x * sx - 1.0, 1.0 - s1.y * sy);
            glEnd();
        }
    }

    void ImagePanel::resizeCanvas(uint32_t w, uint32_t h) {
        if (images_[image_type_paste]) {
            show_error("Can not resize with floating image");
            return;  // wait the user to decide what he's going to do with the floating image
        }
        for (int i = 0; i < image_type_count; i++) {
            if (images_[i]) {
                images_[i] = images_[i]->resizeCanvas(w, h);
            }
        }
        scrollAgain();
    }

    void ImagePanel::resizeImages(uint32_t w, uint32_t h) {
        if (images_[image_type_paste]) {
            show_error("Can not resize with floating image");
            return;  // wait the user to decide what he's going to do with the floating image
        }
        for (int i = 0; i < image_type_count; i++) {
                if (images_[i]) {
                    images_[i] = images_[i]->resizeImage(w, h);
                }
            }
        scrollAgain();
    }

    void ImagePanel::setBrushSize(uint8_t size) {
        if (size > 32)
            brush_size_ = 32;
        else 
            brush_size_ = size;
        should_redraw_ = true;
    }

    uint8_t ImagePanel::getBrushSize() {
        return brush_size_;
    }

    fcoordinate_t ImagePanel::getDrawingCoord() {
        fcoordinate_t r;
        auto ref = get_reference_image();
        if (!ref) {
            return r;
        }
        int refw = ref->w() * zoom_;
        int refh = ref->h() * zoom_;
        r.x = -1;
        r.y = 1;
        if (refw < this->w()) {
            r.x = -(((2.0 / this->w()) * refw) / 2.0);
        }
        if (refh < this->h()) {
            r.y = (((2.0 / this->h()) * refh) / 2.0);
        }
        return r;
    }

    coordinate_t ImagePanel::getDrawingCoordScreen() {
        fcoordinate_t c_gl = getDrawingCoord();
        c_gl.x += 1.0;
        c_gl.y -= 1.0;
        c_gl.y = -c_gl.y;
        coordinate_t r;
        r.x = (w() / 2.0) * c_gl.x;
        r.y = (h() / 2.0) * c_gl.y;
        return r;
    }

    void ImagePanel::draw_buffer(RawImage *img) {
        auto ref = get_reference_image();

        if (!img || !ref) {
            return;
        }

        fcoordinate_t dcoord = getDrawingCoord();

        glRasterPos2f(dcoord.x, dcoord.y);
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

    void ImagePanel::convertToImageCoords(int *x, int *y) {
        RawImage *ref = get_reference_image();
        if (!ref) {
            return;
        }
        coordinate_t dcoord = getDrawingCoordScreen();
        // remove the drawing coord
        *x -= dcoord.x;
        *y -= dcoord.y;
     
        *x /= zoom_;
        *y /= zoom_;
     
        //*x += scroll_x_ ;
        //*y += scroll_y_;
        int xmove, ymove;
        fix_scroll(&xmove, &ymove);
        *x += xmove;
        *y += ymove;
    }

    void ImagePanel::convertToScreenCoords(int *x, int *y) {
        RawImage *ref = get_reference_image();
        if (!ref) {
            return;
        }
        coordinate_t dcoord = getDrawingCoordScreen();
        // include the drawing coord
        int xmove, ymove;
        fix_scroll(&xmove, &ymove);
        *x -= xmove;
        *y -= ymove;
        //*x -= scroll_x_;
        //*y -= scroll_y_;

        *x *= zoom_;
        *y *= zoom_;

        *x += dcoord.x;
        *y += dcoord.y;
    }

    void ImagePanel::fix_scroll(int *xmove, int *ymove) {
        RawImage *ref = get_reference_image();
        if (!ref) {
            *xmove = 0;
            *ymove = 0;
            return; // we should have a reference 
        }
        // the images scrolls from 0,0
        // we wan't zero to be its half
        float half_x = (ref->w() / 2.0);
        float half_y =  (ref->h() / 2.0);
        int scroll_x = scroll_x_ + half_x;
        int scroll_y = scroll_y_ + half_y;
        // now we are at the center of the image
        // we need to scroll down to up left coords of the screen
        half_x = (w() / zoom_ ) / 2.0;
        half_y = (h() / zoom_ ) / 2.0;
        scroll_x -= half_x;
        scroll_y -= half_y;
        *xmove = (scroll_x); 
        *ymove = (scroll_y); 
        if (*xmove < 0) 
            *xmove = 0;
        if (*ymove < 0) 
            *ymove = 0;
    }
    
    void ImagePanel::scrollAgain() {
        setScroll(scroll_x_, scroll_y_);
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

    coordinate_t ImagePanel::getReferenceSize() {
        coordinate_t r;
        auto ref = get_reference_image();
        if (ref) {
            r.x = ref->w();
            r.y = ref->h();
        }
        return r;
    }

    bool ImagePanel::hasReference() {
        if (get_reference_image()) {
            return true;
        }
        return false;
    }

    void ImagePanel::noSelection() {
        if (images_[image_type_paste]) {
            return;
        }
        selection_start_ = selection_end_;
    }

    void ImagePanel::newImage(int w, int h) {
        for (int i = 0; i < image_type_count; i++) {
            images_[i].reset();
        }
        noSelection();
        setZoomLevel(1.0);
        images_[image_type_image].reset(new RawImage(NULL, w, h, dexpert::py::img_rgba, false));
        setScroll(0, 0);
    }

} // namespace dexpert