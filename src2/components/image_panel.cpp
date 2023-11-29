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

    void Layer::refresh() {
        parent_->refresh();
    }

    Layer::~Layer() {
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

    bool Layer::modified() {
        return prior_version_ != version_;
    }

    void Layer::clear_modified() {
        prior_version_ = version_;
    }

    void Layer::set_modified() {
        prior_version_ = version_ - 1;
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
    
    ViewSettings::ViewSettings(ImagePanel *parent): parent_(parent) {

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

    void ViewSettings::remove_layer(size_t position) {
        layers_.erase(layers_.begin() + position);
        refresh();
        publish_event(parent_, event_layer_count_changed, NULL);
    }

    void ViewSettings::clear_layers() {
        name_index_ = 1;
        layers_.clear();
        refresh(true);
        publish_event(parent_, event_layer_count_changed, NULL);
    }

    void ViewSettings::refresh(bool force) {
        parent_->schedule_redraw(force);
    }

    ImageCache::ImageCache() {

    }

    ImageCache::~ImageCache() {

    }

    RawImage *ImageCache::get_cached(float zoom, Layer *layer) {
        layer->clear_modified();
        double sw = (double)layer->w() * zoom;
        double sh = (double)layer->w() * zoom;
        double iw = (double)layer->getImage()->w() * zoom;
        double ih = (double)layer->getImage()->h() * zoom;
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
            }
        }
        return layer->getImage();
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
        view_settings_(this), 
        cache_() {

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
                float z = wheel_delta_ > 0 ? -0.05 : 0.05;
                if (wheel_delta_ != 0) {
                    setZoomLevel(getZoomLevel() + z);
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
            modified = view_settings_.at(i)->modified();
        }

        if (modified) {
            force_redraw_ = false;
            image_->clear(255, 255, 255, 255);
            cache_.clear_hits();
            for (size_t i = 0; i < view_settings_.layer_count(); i++) {
                draw_layer(view_settings_.at(i));
            }
            cache_.gc();
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
        auto img = cache_.get_cached(1.0, layer);
        image_->pasteAt(0, 0, img);
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
        
        int refw = ref->w() * zoom_;
        int refh = ref->h() * zoom_;
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
    }

    void ImagePanel::resize(int x, int y, int w, int h) {
        Fl_Gl_Window::resize(x, y, w, h);
        schedule_redraw(true);
    };

    void ImagePanel::setZoomLevel(float level) {
        zoom_ = level;
        printf("Current zoom %03f\n", zoom_);
        schedule_redraw(true);
    }

    float ImagePanel::getZoomLevel() {
        return zoom_;
    }

} // namespace dfe
