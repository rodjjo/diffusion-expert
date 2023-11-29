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
        void set_modified();
        bool selected();
        void restore_size();
        void scale_size(bool up);
        std::shared_ptr<Layer> duplicate();
    private:
        void refresh(bool force=false);

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
            void get_bounding_box(float zoom, Layer *layer, int *x, int *y, int *w, int *h);
            bool is_layer_visible(float zoom, Layer *layer, int scroll_x, int scroll_y, int view_w, int view_h);
            RawImage *get_cached(float zoom, Layer *layer);
            bool is_modified(Layer *layer);
            void set_scroll(int x, int y);
            int get_scroll_x();
            int get_scroll_y();
        private:
            int scroll_x_ = 0;
            int scrool_y_ = 0;
            std::map<void*, CachedLayer> items_;
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
        Layer* selected_layer();
        size_t selected_layer_index();
        void select(size_t index);
        size_t layer_at_mouse_coord(float zoom, int x, int y);
        void mouse_drag(float zoom, int dx, int dy, int x, int y);
        void mouse_drag_begin();
        void mouse_scale(bool up);
        ImageCache *cache();
        uint16_t getZoom();
        void setZoom(uint16_t value);
        void constraint_scroll(float zoom, int view_w, int view_h, int *sx, int *sy);
        void get_image_area(int *x, int *y, int *w, int *h);
        void duplicate_selected();
    private:
        Layer* add_layer(std::shared_ptr<Layer> l);
        void scroll_again(float old_zoom);
        void compact_image_area();

    private:
        std::vector<std::shared_ptr<Layer> > layers_;
        int drag_begin_x_ = 0;
        int drag_begin_y_ = 0;
        Layer *selected_;
        ImagePanel *parent_;
        size_t name_index_ = 1;
        ImageCache cache_;
        uint16_t zoom_ = 100;
    };

    
        
    class ImagePanel : public Fl_Gl_Window
    {
    public:
        ImagePanel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *unique_title);
        virtual ~ImagePanel();
        ViewSettings *view_settings();
        void resize(int x, int y, int w, int h) override;
        void set_wheel_delta(int16_t delta);
        void hack_window_proc(uintptr_t parent_hwnd);
        float getZoom();
        
    protected:
        // mouse routines
        virtual void mouse_move(bool left_button, bool right_button, int down_x, int down_y, int move_x, int move_y, int from_x, int from_y);
        virtual void mouse_down(bool left_button, bool right_button, int down_x, int down_y);
        virtual void mouse_up(bool left_button, bool right_button, int down_x, int down_y, int up_x, int up_y);

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
        void mouse_drag(int dx, int dy, int x, int y);

    private:
        friend class ViewSettings;
        void schedule_redraw(bool force=false);

    private:
        int16_t wheel_delta_ = 0;
        image_ptr_t image_;
        ViewSettings view_settings_;
        image_ptr_t buffer_;
        bool should_redraw_ = true;
        bool force_redraw_ = false;
        

    private:
        // mouse variables
        bool mouse_down_control_ = false;
        bool mouse_down_shift_ = false;
        bool mouse_down_alt_ = false;
        bool mouse_down_left_ = false;
        bool mouse_down_right_ = false;
        int mouse_down_x_ = 0;
        int mouse_down_y_ = 0;
        int move_last_x_ = 0;
        int move_last_y_ = 0;
        int current_x_ = 0;
        int current_y_ = 0;
        int scroll_px_ = 0;
        int scroll_py_ = 0;
    };

} // namespace dfe
