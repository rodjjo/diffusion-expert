#pragma once

#include <vector>
#include <map>
#include <Fl/Fl_Gl_Window.H>

#include "python/image.h"

namespace dfe
{
    class ImagePanel;
    class ViewSettings;

    class Layer {
      public:
        Layer(ViewSettings *parent, const char *path);
        Layer(ViewSettings *parent, int w, int h, bool transparent);
        Layer(ViewSettings *parent, image_ptr_t image);
        virtual ~Layer();
        RawImage *getImage();
        const char *name();
        void name(const char *value);
        int x();
        int y();
        int w();
        int h();
        void x(int value);
        void y(int value);
        void w(int value);
        void h(int value);
        int version();
        bool modified();
        void clear_modified();
        void set_modified();

    private:
        void refresh();

    private:
        image_ptr_t image_;
        std::string name_;
        ViewSettings *parent_;
        int version_ = 0;
        int prior_version_ = -1;
        int x_ = 0;
        int y_ = 0;
        int w_ = 1;
        int h_ = 1;
    };
    
    class ViewSettings {
    public:
        ViewSettings(ImagePanel *parent);
        virtual ~ViewSettings();
        size_t layer_count();
        Layer* add_layer(const char *path);
        Layer* add_layer(int w, int h, bool transparent);
        Layer* add_layer(image_ptr_t image);
        Layer* at(size_t position);
        void remove_layer(size_t position);
        void refresh(bool force=false);
        void clear_layers();
    private:
        Layer* add_layer(std::shared_ptr<Layer> l);
    private:
        std::vector<std::shared_ptr<Layer> > layers_;
        ImagePanel *parent_;
        size_t name_index_ = 1;
    };

    class CachedLayer {
        public:
            bool hit;
            size_t version;
            image_ptr_t cache;
    };

    class ImageCache {
        public:
            ImageCache();
            virtual ~ImageCache();
            void clear_hits();
            void gc();
            RawImage *get_cached(float zoom, Layer *layer);
        private:
            std::map<void*, CachedLayer> items_;
    };
        
    class ImagePanel : public Fl_Gl_Window
    {
    public:
        ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *unique_title);
        virtual ~ImagePanel();
        ViewSettings *view_settings();
        void resize(int x, int y, int w, int h) override;
        void set_wheel_delta(int16_t delta);
        void setZoomLevel(float level);
        float getZoomLevel();
        void hack_window_proc(uintptr_t parent_hwnd);

    protected:
        int handle(int event) override;
        void draw() override;
        void draw_overlay() override;

    private:
#ifdef _WIN32
        static LRESULT dfe_window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        WNDPROC original_wnd_proc_;
        fl_uintptr_t native_hwnd_;
#endif
        void draw_layer(Layer *layer);
        static void imageRefresh(void *cbdata);
        void imageRefresh();
        void getDrawingCoord(float &x, float &y);
    private:
        friend class ViewSettings;
        void schedule_redraw(bool force=false);
    private:
        int16_t wheel_delta_ = 0;
        image_ptr_t image_;
        ImageCache cache_;
        ViewSettings view_settings_;
        image_ptr_t buffer_;
        bool should_redraw_ = true;
        bool force_redraw_ = false;
        float zoom_ = 1.0;
    };

} // namespace dfe
