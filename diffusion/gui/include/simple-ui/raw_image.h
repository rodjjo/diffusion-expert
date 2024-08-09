#pragma once

#include <inttypes.h>
#include <memory>

#define RGBA_TO_COLOR(r, g, b, a) ((r) << 24) | ((g) << 16) | ((b) << 8)  | (a)
#define RGBA_R(color) (((color) >> 24) & 255)
#define RGBA_G(color) (((color) >> 16) & 255)
#define RGBA_B(color) (((color) >> 8) & 255)
#define RGBA_A(color) ((color) & 255)

namespace sf {
    class Image;
}

namespace dfe_ui
{

class RawImage;

typedef std::shared_ptr<RawImage> image_ptr_t;

class RawImage {
        void init(uint32_t w, uint32_t h, bool transparent);
        RawImage(uint32_t w, uint32_t h, bool transparent);
    public:
        RawImage(uint32_t w, uint32_t h, uint32_t fill_color, bool transparent);
        virtual ~RawImage();
        static image_ptr_t from_image(const sf::Image *img);
        static image_ptr_t from_file(const std::wstring &path);
        bool is_rgba();
        size_t buffer_size();
        unsigned char *buffer();

        uint32_t w();
        uint32_t h();
        uint32_t size();
        uint8_t bytes_per_pixel();
        uint32_t stride();
        uint64_t version();

        void draw_image_at(int x, int y, RawImage *image);
        void draw_image_at(int x, int y, RawImage *mask, RawImage *image);
        void draw_image_at(int x, int y, int w, int h, RawImage *image);
        void draw_circle(int x, int y, int radius, uint32_t color, uint32_t bgcolor, bool clear);
        void draw_circle(int x, int y, int radius, bool clear);

        image_ptr_t duplicate();
        image_ptr_t to_rgba();
        image_ptr_t to_rgb();
        image_ptr_t to_blurred_image(uint32_t size);
        image_ptr_t to_resized_image(uint32_t w, uint32_t h);
        image_ptr_t to_resized_canvas(uint32_t w, uint32_t h);
        image_ptr_t to_resized_multiple_of_8x8();
        image_ptr_t to_resized_down_use_alpha();
        image_ptr_t to_flipped_image(bool vertically);
        image_ptr_t to_inverted_colors(RawImage *mask);

        void save(const std::wstring& path);
        
    private:
        std::shared_ptr<unsigned char> m_buffer;
        uint32_t m_w;
        uint32_t m_h;
        uint32_t m_size;
        uint8_t m_bytes_per_pixel;
        uint32_t m_stride;
        bool m_rgba;
        uint64_t m_version = 0;
};
    
} // namespace dfe_ui

