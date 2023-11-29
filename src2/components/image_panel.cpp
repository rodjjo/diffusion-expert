#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "misc/utils.h"
#include "python/routines.h"
#include "messagebus/messagebus.h"

#include "components/image_panel.h"

namespace dfe
{
    namespace {
        const int gl_format[dfe::py::img_format_count] = {
            GL_LUMINANCE,
            GL_RGB,
            GL_RGBA
        };

        std::map<fl_uintptr_t, void*> window_user_data;

        uint8_t red_color[4] = {255, 0, 0, 255};
        uint8_t gray_color[4] = {128, 128, 128, 255};
    }

    Layer::Layer(ViewSettings *parent, const char *path) : parent_(parent), image_(py::open_image(path)) {
        if (image_) {
            w_ = image_->w();
            h_ = image_->h();
        }
    }

    Layer::Layer(ViewSettings *parent, int w, int h, bool transparent) : parent_(parent), image_(py::newImage(w, h, transparent))  {
        if (image_) {
            w_ = image_->w();
            h_ = image_->h();
        }
    }

    Layer::Layer(ViewSettings *parent, image_ptr_t image) : parent_(parent), image_(image->duplicate()) {
        if (image_) {
            w_ = image_->w();
            h_ = image_->h();
        }
    }

    void Layer::refresh(bool force) {
        parent_->refresh(force);
    }

    Layer::~Layer() {
    }

    std::shared_ptr<Layer> Layer::duplicate() {
        auto l = std::make_shared<Layer>(parent_, this->image_->duplicate());
        l->x(x());
        l->y(y());
        l->w(w());
        l->h(h());
        return l;
    }

    RawImage *Layer::getImage() {
        return image_.get();
    }

    int Layer::x() {
        return x_;
    }

    int Layer::y() {
        return y_;
    }

    int Layer::w() {
        return w_;
    }

    int Layer::h() {
        return h_;
    }

    bool Layer::selected() {
        return parent_->selected_layer() == this && parent_->layer_count() > 1;
    }

    const char *Layer::name() {
        return name_.c_str();
    }

    void Layer::name(const char *value) {
        name_ = value;
    }

    void Layer::x(int value) {
        if (value != x_) {
            x_ = value;
            ++version_;
            refresh();
        }
    }

    void Layer::set_modified() {
        ++version_;
    }

    void Layer::y(int value) {
        if (value != y_) {
            y_ = value;
            ++version_;
            refresh();
        }
    }

    void Layer::w(int value) {
        if (value != w_) {
            w_ = value;
            ++version_;
            refresh();
        }
    }

    void Layer::h(int value) {
        if (value != h_) {
            h_ = value;
            ++version_;
            refresh();
        }
    }

    int Layer::version() {
        return version_;
    }

    void Layer::restore_size() {
        if (image_) {
            w_ = image_->w();
            h_ = image_->h();
            ++version_;
            refresh(true);
        }
    }

    void Layer::scale_size(bool up) {
        if (image_) {
            if (image_->w() * 4.0 < w_ || image_->h() * 4.0 < h_)  {
                w_ = image_->w() * 4.0;
                h_ = image_->h() * 4.0;
            }
            if (image_->w() * 0.10 > w_ || image_->h() * 0.10 > h_) {
                w_ = image_->w() * 0.10;
                h_ = image_->h() * 0.10;
            }
            float five_x = w_ * 0.05;
            float five_y = h_ * 0.05;
            if (up) {
                w_ += five_x;
                h_ += five_y;
                x_ -= five_x / 2.0;
                y_ -= five_y / 2.0;
            } else {
                w_ -= five_x;
                h_ -= five_y;
                x_ += five_x / 2.0;
                y_ += five_y / 2.0;
            }
            ++version_;
            refresh(true);
        }
    }
    
    ViewSettings::ViewSettings(ImagePanel *parent): parent_(parent), cache_() {

    }

    ViewSettings::~ViewSettings() {

    }

    size_t ViewSettings::layer_count() {
        return layers_.size();
    }

    Layer* ViewSettings::add_layer(std::shared_ptr<Layer> l) {
        if (l->getImage()) {
            if (strlen(l->name()) == 0) {
                char buffer[128] = "";
                sprintf(buffer, "Layer %03d", name_index_);
                l->name(buffer);
            }
            name_index_ += 1;
            layers_.push_back(l);
            refresh();
            publish_event(parent_, event_layer_count_changed, NULL);
            return l.get();
        }
        return NULL;
    }

    Layer* ViewSettings::add_layer(const char *path) {
        return add_layer(std::make_shared<Layer>(this, path));
    }

    Layer* ViewSettings::add_layer(int w, int h, bool transparent) {
        return add_layer(std::make_shared<Layer>(this, w, h, transparent));
    }

    Layer* ViewSettings::add_layer(image_ptr_t image) {
        return add_layer(std::make_shared<Layer>(this, image));
    }

    Layer* ViewSettings::at(size_t position) {
        return layers_.at(position).get();
    }

    void ViewSettings::duplicate_selected() {
        if (selected_) {
            add_layer(selected_->duplicate());
        }
    }

    void ViewSettings::remove_layer(size_t position) {
        if (position > layers_.size()) {
            return;
        }
        if (selected_ == layers_.at(position).get()) {
            selected_ = NULL;
        }
        layers_.erase(layers_.begin() + position);
        if (layers_.empty()) {
            cache_.set_scroll(0, 0);
        }
        refresh();
        publish_event(parent_, event_layer_count_changed, NULL);
    }

    void ViewSettings::select(size_t index) {
        if (index < layers_.size()) {
            if (selected_ != layers_.at(index).get()) {
                selected_ = layers_.at(index).get();
                refresh(true);
                publish_event(parent_, event_layer_selected, selected_);
            }
        }
    }

    size_t ViewSettings::selected_layer_index() {
        for (size_t i = 0; i < layers_.size(); i++) {
            if (layers_[i].get() == selected_) {
                return i;
            }
        }
        return 0;
    }

    ImageCache *ViewSettings::cache() {
        return &cache_;
    }

    uint16_t ViewSettings::getZoom() {
        return zoom_;
    }

    void ViewSettings::setZoom(uint16_t value) {
        if (value < 10) {
            value = 10;
        } else if (value > 200) {
            value = 200;
        }
        if (value == zoom_) {
            return;
        }
        float old_zoom = zoom_;
        zoom_ = value;
        scroll_again(old_zoom);
        refresh(true);
    }

    void ViewSettings::clear_layers() {
        name_index_ = 1;
        selected_ = NULL;
        layers_.clear();
        cache_.set_scroll(0, 0);
        refresh(true);
        publish_event(parent_, event_layer_count_changed, NULL);
    }

    Layer* ViewSettings::selected_layer() {
        return selected_;
    }

    void ViewSettings::refresh(bool force) {
        parent_->schedule_redraw(force);
    }

    size_t ViewSettings::layer_at_mouse_coord(float zoom, int x, int y) {
        if (zoom != 0) {
            float mx = cache_.get_scroll_x() * zoom + x / zoom;
            float my = cache_.get_scroll_y() * zoom + y / zoom;
            Layer *l;
            for (size_t i = layers_.size(); i > 0; i--) {
                l = layers_[i - 1].get();
                if (l->x() < mx && mx <  l->x() + l->w() 
                    && l->y() < my && my <  l->y() + l->h() ) {
                    return i - 1;
                }
            }
        }
        return (size_t) -1;
    }

    void ViewSettings::mouse_drag(float zoom, int dx, int dy, int x, int y) {
        if (zoom == 0 || !selected_) 
            return;
        float mx = x / zoom;
        float my = y / zoom;
        float mdx = dx / zoom;
        float mdy = dy / zoom;
        float dragx = mx - mdx;
        float dragy = my - mdy;
        selected_->x(drag_begin_x_ + dragx);
        selected_->y(drag_begin_y_ + dragy);
        compact_image_area();
        parent_->schedule_redraw(true);
    }

    void ViewSettings::compact_image_area() {
        /*
            This function ensure that at least one layer has the position (0, 0)
        */
        int x, y, w, h;
        get_image_area(&x, &y, &w, &h);
        int add_x = -x;
        int add_y = -y;
        for (auto & l : layers_) {
            l->x(l->x() + add_x);
            l->y(l->y() + add_y);
        }
        parent_->schedule_redraw(true);
    }

    void ViewSettings::mouse_drag_begin() {
        if (selected_) {
            drag_begin_x_ = selected_->x();
            drag_begin_y_ = selected_->y();
        }
    }

    void ViewSettings::mouse_scale(bool up) {
        if (selected_) {
            selected_->scale_size(up);
            compact_image_area();
        } 
    }

    void ViewSettings::get_image_area(int *x, int *y, int *w, int *h) {
        int max_x = -32000;
        int max_y = -32000;
        int min_x = 32000;
        int min_y = 32000;

        for (auto & l: layers_) {
            if (l->x() + l->w() > max_x) {
                max_x = l->x() + l->w();
            }
            if (l->y() + l->h() > max_y) {
                max_y = l->y() + l->h();
            }
            if (l->x() < min_x) {
                min_x = l->x();
            }
            if (l->y() < min_y) {
                min_y = l->y();
            }
        }

        *x = min_x;
        *y = min_y;
        *w = max_x - min_x;
        *h = max_y - min_y;
    }

    void ViewSettings::constraint_scroll(float zoom, int view_w, int view_h, int *sx, int *sy) {
        if (zoom == 0) {
            return;
        }
        int x, y, w, h;
        get_image_area(&x, &y, &w, &h);
        
        int half_view_x = (view_w / 2) / zoom;
        int half_view_y = (view_h / 2) / zoom;

        int scroll_size_x = (w) - half_view_x;
        int scroll_size_y = (h) - half_view_y;

        if (*sx > half_view_x) {
            *sx = half_view_x;
        }
        if (*sy > half_view_y) {
            *sy = half_view_y;
        }
        if (*sx < -scroll_size_x) {
            *sx = -scroll_size_x;
        }
        if (*sy < -scroll_size_y) {
            *sy = -scroll_size_y;
        }
       
        printf("Scroll coordinate %d x %d max(%d x %d) zoom %03f\n", *sx, *sy, scroll_size_x, scroll_size_y, zoom);
    }
    
    void ViewSettings::scroll_again(float old_zoom) {
        return;
        if (old_zoom != 0 && getZoom() != 0) {
            float diff = getZoom() / old_zoom;
            cache_.set_scroll(
                cache_.get_scroll_x() * diff,
                cache_.get_scroll_y() * diff
            );
        }
    }

    ImageCache::ImageCache() {

    }

    ImageCache::~ImageCache() {

    }

    void ImageCache::set_scroll(int x, int y) {
        scroll_x_ = x;
        scrool_y_ = y;
    }

    int ImageCache::get_scroll_x() {
        return scroll_x_;
    }

    int ImageCache::get_scroll_y() {
        return scrool_y_;
    }

    void ImageCache::get_bounding_box(float zoom, Layer *layer, int *x, int *y, int *w, int *h) {
        *w = round((float)layer->w() * zoom);
        *h = round((float)layer->h() * zoom);
        *x = round((float)(scroll_x_ + layer->x()) * zoom);
        *y = round((float)(scrool_y_ + layer->y()) * zoom);
    }

    bool ImageCache::is_layer_visible(float zoom, Layer *layer, int scroll_x, int scroll_y, int view_w, int view_h) {
        if (zoom == 0) {
            return false;
        }
        int x, y, w, h;
        get_bounding_box(zoom, layer,  &x, &y, &w, &h);
        return rectRect(x, y, w, h, 0, 0, view_w, view_h);
    }

    RawImage *ImageCache::get_cached(float zoom, Layer *layer) {
        int _unused;
        int sw, sh;
        get_bounding_box(zoom, layer, &_unused, &_unused, &sw, &sh);
        int iw = (float)layer->getImage()->w() * zoom;
        int ih = (float)layer->getImage()->h() * zoom;
        if (sw * 0.8 < iw ||
            sh * 0.8 < ih || zoom < 0.8) {
            auto it = items_.find(layer);
            RawImage *result = NULL;
            if (it != items_.end()) {
                if (it->second.version == layer->version() && it->second.cache) {
                    result = it->second.cache.get();
                    it->second.hit = true;
                }
            } 
            if (result == NULL) {
                CachedLayer cl;
                cl.version = layer->version();
                cl.hit = true;
                cl.cache = layer->getImage()->resizeImage(sw, sh);
                items_[layer] = cl;
                result  = cl.cache.get();
            }
            return result;
        }

        return layer->getImage();
    }

    bool ImageCache::is_modified(Layer *layer) {
        auto it = items_.find(layer);
        if (it != items_.end()) {
            return it->second.version != layer->version();
        }
        return true;
    }

    void ImageCache::clear_hits() {
        for (auto & i : items_) {
            i.second.hit = false;
        }
    }

    void ImageCache::gc() {
        // clear from cache all images that has no hits
        for (auto it = items_.begin(); it != items_.end(); ) {
            if (it->second.hit) {
                it++;
            } else {
                it = items_.erase(it);
            }
        }
    }

    ImagePanel::ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *unique_title) : 
        Fl_Gl_Window(x, y, w, h, unique_title), 
        view_settings_(this) {

        Fl::add_timeout(0.01, ImagePanel::imageRefresh, this);
    }

    void ImagePanel::hack_window_proc(uintptr_t parent_hwnd) {
#ifdef _WIN32
        /*
        FLTK does not handle mouse wheel event weel.
        I'm hacking the window proc so.
        */
        native_hwnd_ = find_current_thread_window(this->label(), parent_hwnd);
        if (native_hwnd_) {
            window_user_data[native_hwnd_] = this;
            auto hwnd = (HWND)native_hwnd_;
            original_wnd_proc_ = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)&ImagePanel::dfe_window_proc);
        }
#endif
    }

    ImagePanel::~ImagePanel() {
        Fl::remove_timeout(ImagePanel::imageRefresh, this);
        #ifdef _WIN32
        if (native_hwnd_) {
            SetWindowLongPtrW((HWND)native_hwnd_, GWLP_WNDPROC, (LONG_PTR)original_wnd_proc_);
            window_user_data.erase(native_hwnd_);
        }
        #endif
    }

#ifdef _WIN32
    // fltk does not handle mouse scroll event well.
    LRESULT ImagePanel::dfe_window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        ImagePanel *_this = (ImagePanel *) window_user_data[(fl_uintptr_t)hWnd];
        if (msg == WM_MOUSEWHEEL) {
            _this->set_wheel_delta(GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return _this->original_wnd_proc_(hWnd, msg, wParam, lParam);
    }
#endif
    
    void ImagePanel::imageRefresh(void *cbdata) {
        ((ImagePanel *) cbdata)->imageRefresh();
        Fl::repeat_timeout(0.018, ImagePanel::imageRefresh, cbdata);
    }

    void ImagePanel::imageRefresh() {
        if (should_redraw_) {
            should_redraw_ = false;
            if (!image_ || image_->w() != w() || image_->h() != h()) {
                image_ = py::newImage(w(), h(), false);
                should_redraw_ = true;
                force_redraw_ = true;
                return;
            }
            redraw();
        }
    }

    ViewSettings *ImagePanel::view_settings() {
        return &view_settings_;
    }

    void ImagePanel::set_wheel_delta(int16_t delta) {
        wheel_delta_ = delta;
    }

    int ImagePanel::handle(int event)
    {
        switch (event) {
            case FL_MOUSEWHEEL: {
                int16_t z = wheel_delta_ > 0 ? -10 : 10;
                if (wheel_delta_ != 0) {
                    bool control_pressed = Fl::event_command() != 0;
                    bool shift_pressed = Fl::event_shift() != 0;
                    bool alt_pressed = Fl::event_alt() != 0;
                    if (!control_pressed && !shift_pressed && !alt_pressed) {
                        view_settings_.setZoom(view_settings_.getZoom() + z);
                    } else if (!control_pressed && shift_pressed && !alt_pressed) {
                        view_settings_.mouse_scale(z > 0);
                    }
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
                    mouse_down_alt_ = Fl::event_alt() != 0;
                    mouse_down_x_ = Fl::event_x();
                    mouse_down_y_ = Fl::event_y();
                    scroll_px_ = view_settings_.cache()->get_scroll_x();
                    scroll_py_ = view_settings_.cache()->get_scroll_y();
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
                    // mouse_up(mouse_down_left_, mouse_down_right_, mouse_down_x_, mouse_down_y_, Fl::event_x(), Fl::event_y());
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

    void ImagePanel::draw() {
        if (!valid())
        {
            valid(1);
            glLoadIdentity();
            glViewport(0, 0, this->w(), this->h());
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!image_) {
            return;
        }
        
        bool modified = force_redraw_;
        for (size_t i = 0; i < view_settings_.layer_count() && !modified; i++) {
            modified = view_settings_.cache()->is_modified(view_settings_.at(i));
        }

        if (modified) {
            force_redraw_ = false;
            image_->clear(255, 255, 255, 255);
            view_settings_.cache()->clear_hits();
            for (size_t i = 0; i < view_settings_.layer_count(); i++) {
                draw_layer(view_settings_.at(i));
            }
            int ix, iy, iw, ih;
            view_settings_.get_image_area(&ix, &iy, &iw, &ih);
            ix += view_settings_.cache()->get_scroll_x();
            iy += view_settings_.cache()->get_scroll_y();
            float zoom = getZoom();
            ix *= zoom;
            iy *= zoom;
            iw *= zoom;
            ih *= zoom;
            image_->rectangle(ix, iy, iw, ih, gray_color);
            view_settings_.cache()->gc();
        }

        glRasterPos2f(-1, 1);
        glPixelZoom(1, -1);

        if (image_->w() % 4 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        else
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glDrawPixels(image_->w(), image_->h(), gl_format[image_->format()], GL_UNSIGNED_BYTE, image_->buffer());

        glRasterPos2f(0.0f, 0.0f);
        glPixelZoom(1.0f, 1.0f);

        //blur_gl_contents(this->w(), this->h(), current_x_, current_y_);
    }

    void ImagePanel::draw_layer(Layer *layer) {
        float zoom = getZoom();
        if (!view_settings_.cache()->is_layer_visible(zoom, layer, 0, 0, image_->w(), image_->h())) {
            return;
        }
        auto img = view_settings_.cache()->get_cached(zoom, layer);
        int x, y, w, h;
        view_settings_.cache()->get_bounding_box(zoom, layer, &x, &y, &w, &h);
        image_->pasteAt(x, y, img);
        if (layer->selected()) {
            image_->rectangle(x, y, w, h, red_color);
        }
    }

    void ImagePanel::getDrawingCoord(float &x, float &y) {
        x = -1;
        y = 1;
        if (view_settings_.layer_count() < 1) {
            return;
        }

        auto ref = view_settings_.at(0)->getImage(); 
        if (!ref) {
            return;
        }
        
        int refw = ref->w() * getZoom();
        int refh = ref->h() * getZoom();
        x = -1;
        y = 1;
        if (refw < this->w()) {
            x = -(((2.0 / this->w()) * refw) / 2.0);
        }
        if (refh < this->h()) {
            y = (((2.0 / this->h()) * refh) / 2.0);
        }
    }

    void ImagePanel::draw_overlay() {
        //Fl_Gl_Window::draw_overlay();
    }

    void ImagePanel::schedule_redraw(bool force) {
        force_redraw_ = force_redraw_ || force;
        should_redraw_ = true;
        if (force_redraw_) {
            for (size_t i = 0; i < view_settings_.layer_count(); i++) {
                view_settings_.at(i)->set_modified();
            }
        }
    }

    void ImagePanel::resize(int x, int y, int w, int h) {
        Fl_Gl_Window::resize(x, y, w, h);
        schedule_redraw(true);
    };

    float ImagePanel::getZoom() {
        return view_settings_.getZoom() * 0.01;
    }
    
    void ImagePanel::mouse_drag(int dx, int dy, int x, int y) {
        dx = (x - dx) / getZoom();
        dy = (y - dy) / getZoom();
        auto sx = scroll_px_ + dx;
        auto sy = scroll_py_ + dy;
        view_settings_.constraint_scroll(getZoom(), image_->w(), image_->h(), &sx, &sy);
        view_settings_.cache()->set_scroll(sx,  sy);
        schedule_redraw(true);
    }

    void ImagePanel::mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y, int from_x, int from_y) {
        if (mouse_down_control_ && left_button) {
            // control was pressed during mouse down, so lets change the scroll 
            mouse_drag(down_x, down_y, move_x, move_y);
            down_x = move_x;
            down_y = move_y;
            return;
        }

        if (right_button && !mouse_down_control_ && !mouse_down_shift_) {
            view_settings_.mouse_drag(getZoom(), down_x, down_y, move_x, move_y);
        }
    }

    void ImagePanel::mouse_down(bool left_button, bool right_button, int down_x, int down_y) {
        if (!mouse_down_control_ && (left_button || right_button)) {
            // select the layer at the mouse coordinates
            auto index = view_settings_.layer_at_mouse_coord(getZoom(), down_x, down_y);
            view_settings_.select(index);
        }

        if (right_button && !mouse_down_control_ && !mouse_down_shift_) {
            view_settings_.mouse_drag_begin();
        }
    }

    void ImagePanel::mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y) {
        if (right_button && !mouse_down_control_) {
            
        }
    }

} // namespace dfe
