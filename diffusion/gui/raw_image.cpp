#include <stdlib.h>
#include <string.h>

#include <CImg.h>

#include "simple-ui/raw_image.h"

using namespace cimg_library;

namespace dfe_ui
{

void RawImage::init(uint32_t w, uint32_t h, bool rgba) {
    size_t pixel_count = w * h;
    m_w = w;
    m_h = h;
    m_rgba = rgba;
    m_bytes_per_pixel = (rgba ? 4 : 3);
    m_stride = m_w * m_bytes_per_pixel;
    m_size = pixel_count * m_bytes_per_pixel;
    m_buffer.reset((unsigned char*)malloc(m_size));
}

RawImage::RawImage(uint32_t w, uint32_t h, bool rgba) {
    init(w, h, rgba);
}

RawImage::RawImage(uint32_t w, uint32_t h, uint32_t fill_color, bool rgba) {
    size_t pixel_count = w * h;
    init(w, h, rgba);
    if (rgba) {
        typedef unsigned char transp_buffer_t[4];
        uint8_t r = (fill_color >> 3) & 255;
        uint8_t g = (fill_color >> 2) & 255;
        uint8_t b = (fill_color >> 1) & 255; 
        uint8_t a = fill_color & 255;
        unsigned char *buffer = m_buffer.get();
        for (size_t i = 0; i < pixel_count; i++) {
            *buffer = r; buffer++;
            *buffer = g; buffer++;
            *buffer = b; buffer++;
            *buffer = a; buffer++;
        }
    } else {
        typedef unsigned char transp_buffer_t[4];
        uint8_t r = (fill_color >> 3) & 255;
        uint8_t g = (fill_color >> 2) & 255;
        uint8_t b = (fill_color >> 1) & 255; 
        unsigned char *buffer = m_buffer.get();
        for (size_t i = 0; i < pixel_count; i++) {
            *buffer = r; buffer++;
            *buffer = g; buffer++;
            *buffer = b; buffer++;
        }
    }
}


RawImage::~RawImage() {
}

uint32_t RawImage::w() {
    return m_w;
}

uint32_t RawImage::h() {
    return m_h;
}

uint32_t RawImage::size() {
    return m_size;
}

uint8_t RawImage::bytes_per_pixel() {
    return m_bytes_per_pixel;
}

uint32_t RawImage::stride() {
    return m_stride;
}

bool RawImage::is_rgba() {
    return m_rgba;
}

size_t RawImage::buffer_size() {
    return m_size;
}

unsigned char *RawImage::buffer() {
    return m_buffer.get();
}

image_ptr_t RawImage::duplicate() {
    std::shared_ptr<RawImage> result(new RawImage(this->m_w, this->m_h, this->m_rgba));
    memcpy(result->m_buffer.get(), this->m_buffer.get(), this->m_size);
    return result;
}

image_ptr_t RawImage::to_rgba() {
    if (m_rgba) {
        return duplicate();
    }
    size_t pixel_count = m_w * m_h;
    std::shared_ptr<RawImage> result(new RawImage(this->m_w, this->m_h, this->m_rgba));
    unsigned char *src = this->m_buffer.get();
    unsigned char *dest = result->m_buffer.get();
    for (size_t i = 0; i < pixel_count; i++) {
        *dest = *src; src++; dest++;
        *dest = *src; src++; dest++;
        *dest = *src; src++; dest++;
        *dest = 255; // alpha chanel
        dest++;
    }
    return result;
}

image_ptr_t RawImage::to_rgb() {
    if (!m_rgba) {
        return duplicate();
    }
    size_t pixel_count = m_w * m_h;
    std::shared_ptr<RawImage> result(new RawImage(this->m_w, this->m_h, this->m_rgba));
    unsigned char *src = this->m_buffer.get();
    unsigned char *dest = result->m_buffer.get();
    for (size_t i = 0; i < pixel_count; i++) {
        *dest = *src; src++; dest++;
        *dest = *src; src++; dest++;
        *dest = *src; src++; dest++;
        src++; // skip alpha channel
    }
    return result;
}

/// @brief Apply a blur to the image
/// @param size The size of blur
/// @return A copy of the image with blur applied to it
image_ptr_t RawImage::to_blurred_image(uint32_t size) {
    image_ptr_t result = this->duplicate();
    CImg<unsigned char> self(result->m_buffer.get(), m_rgba ? 4 : 3, result->m_w, result->m_h, 1, true);
    self.permute_axes("yzcx");
    self.blur(size, size, 0.0, 1, true);
    self.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::to_resized_image(uint32_t w, uint32_t h) {
    image_ptr_t result(new RawImage(w, h, m_rgba));
    CImg<unsigned char> src(this->m_buffer.get(), m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    CImg<unsigned char> self(result->m_buffer.get(), m_rgba ? 4 : 3, result->m_w, result->m_h, 1, true);
    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    self.draw_image(0, 0, src.get_resize(w, h));
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::to_resized_canvas(uint32_t w, uint32_t h) {
    image_ptr_t result(new RawImage(w, h, m_rgba));
    CImg<unsigned char> src(this->buffer(), m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    CImg<unsigned char> self(result->buffer(), m_rgba ? 4 : 3, result->m_w, result->m_h, 1, true);
    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    self.draw_image(0, 0, src);
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::to_resized_down_use_alpha() {
    /*
        Return the image resized to the minimal size where a aplha value is present in the image.
    */
    if (!m_rgba) {
        return duplicate();
    }
    unsigned char *src = m_buffer.get();
    int min_y = m_h;
    int min_x = m_w;
    int max_x = 0;
    int max_y = 0;
    for (int y = 0; y < m_h; y++) {
        for (int x = 0; x < m_w; x++) {
            if (src[3] != 0) { // alpha != 0
                if (min_x > x) {
                    min_x = x;
                }
                if (min_y > y) {
                    min_y = y;
                }
                if (max_x < x) {
                    max_x = x;
                }
                if (max_y < y) {
                    max_y = y;
                }
            }
            src += 4;
        }
    }

    if (max_x == 0 || max_y == 0) {
        return duplicate();
    }

    int ww = max_x - min_x;
    int hh = max_y - min_y;
    image_ptr_t r;
    r.reset(new RawImage(ww, hh, true));
    src = m_buffer.get();
    unsigned char *target = r->m_buffer.get();
    int source_stride = m_w * 4;
    int target_stride = ww * 4;
    src += (source_stride * min_y) + min_x * 4;
    for (int y = min_y; y < max_y; y++) {
        memcpy(target, src, target_stride);
        src += source_stride;
        target += target_stride;
    }
    return r;
}

image_ptr_t RawImage::to_flipped_image(bool vertically) {
    image_ptr_t result = this->duplicate();
    CImg<unsigned char> self(result->m_buffer.get(), m_rgba ? 4: 3, result->m_w, result->m_h, 1, true);
    self.permute_axes("yzcx");
    if (vertically) {
        self.mirror("y");
    } else {
        self.mirror("x");
    }
    self.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::to_resized_multiple_of_8x8() {
    int diff_w = this->m_w % 8;
    int diff_h = this->m_h % 8;

    if (diff_w = 0 && diff_h == 0) {
        return duplicate();
    }
    if (diff_w > 0) diff_w = 8 - diff_w;
    if (diff_h > 0) diff_h = 8 - diff_h;
    auto result = this->to_resized_image(this->m_w + diff_w, this->m_h + diff_h);
    result->draw_image_at(0, 0, this);
    return result;
}

image_ptr_t RawImage::to_inverted_colors(RawImage *mask) {
    /*
        This fucntion takes the image mask, duplicate it, then draw the current image using the mask.
        The resulting image have the colors inverted.
    */
    if (!mask->m_rgba) {
        return image_ptr_t();
    }

    image_ptr_t result = mask->duplicate();
    CImg<unsigned char> src(this->m_buffer.get(), this->m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    CImg<unsigned char> img(result->m_buffer.get(), result->m_rgba ? 4: 3, result->m_w, result->m_h, 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    auto resized = src.get_resize(result->m_w, result->m_h);
    img.draw_image(0, 0, 0, 0, resized, img.get_shared_channel(3), 1, 255);
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");

    unsigned char *p = result->m_buffer.get();
    for (int i = 0; i < result->m_size; i += 4) {
        *p = 255 - *p; ++p;
        *p = 255 - *p; ++p;
        *p = 255 - *p; ++p;
        ++p;
    }

    return result;
}

void RawImage::draw_image_at(int x, int y, RawImage *image) {
    CImg<unsigned char> src(image->m_buffer.get(), image->m_rgba ? 4 : 3, image->m_w, image->m_h, 1, true);
    CImg<unsigned char> img(this->m_buffer.get(), this->m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    if (image->m_rgba) {
        img.draw_image(x, y, 0, 0, src, src.get_shared_channel(3), 1, 255);
    } else {
        img.draw_image(x, y, src);
    }
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::draw_image_at(int x, int y, RawImage *mask, RawImage *image) {
    CImg<unsigned char> src(image->m_buffer.get(), image->m_rgba ? 4 : 3, image->m_w, image->m_h, 1, true);
    CImg<unsigned char> msk(mask->m_buffer.get(), mask->m_rgba ? 4 : 3, mask->m_w, mask->m_h, 1, true);
    CImg<unsigned char> img(this->m_buffer.get(), this->m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    msk.permute_axes("yzcx");
    if (mask->m_rgba) {
        img.draw_image(x, y, 0, 0, src, msk.get_channel(3), 1, 255);
    } else {
        img.draw_image(x, y, 0, 0, src, msk, 1, 255);
    }
    msk.permute_axes("cxyz");
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::draw_image_at(int x, int y, int w, int h, RawImage *image) {
    CImg<unsigned char> src(image->m_buffer.get(), image->m_rgba ? 4 : 3, image->m_w, image->m_h, 1, true);
    CImg<unsigned char> img(this->m_buffer.get(), this->m_rgba ? 4 : 3, this->m_w, this->m_h, 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    if (image->m_rgba) {
        auto resized = src.get_resize(w, h);
        img.draw_image(x, y, 0, 0, resized, resized.get_shared_channel(3), 1, 255);
    } else {
        img.draw_image(x, y, src.get_resize(w, h));
    }
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

} // namespace dfe_ui
