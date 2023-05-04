/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_RAW_IMAGE_H_
#define SRC_PYTHON_RAW_IMAGE_H_

#include <memory>
#include <Python.h>
#include <pybind11/embed.h> 

namespace py11 = pybind11;

namespace dexpert {
namespace py {

typedef enum {
    img_gray_8bit,
    img_rgb,
    img_rgba,
    // keep img_format_count at the end
    img_format_count
} image_format_t;


class RawImage;
typedef std::shared_ptr<RawImage> image_ptr_t;

class RawImage {
 public:
    RawImage(const unsigned char *buffer, uint32_t w, uint32_t h, image_format_t format, bool fill_transparent=true);
    virtual ~RawImage();
    void toPyDict(py11::dict &image);
    const unsigned char *buffer();
    image_format_t format();
    uint32_t h();
    uint32_t w();
    size_t getVersion();
    void incVersion();
    void pasteFill(RawImage *image);
    void pasteFrom(int x, int y, float zoom, RawImage *image);
    void pasteAt(int x, int y, RawImage *image);
    void pasteAt(int x, int y, RawImage *mask, RawImage *image);
    void pasteAt(int x, int y, int w, int h, RawImage *image);
    
    image_ptr_t duplicate();
    image_ptr_t removeBackground(bool white);
    image_ptr_t removeAlpha();
    image_ptr_t resizeCanvas(uint32_t x, uint32_t y);
    image_ptr_t resizeImage(uint32_t x, uint32_t y);
    image_ptr_t getCrop(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    image_ptr_t ensureMultipleOf8();
    image_ptr_t resizeLeft(int value);
    image_ptr_t resizeRight(int value);
    image_ptr_t resizeTop(int value);
    image_ptr_t resizeBottom(int value);
    image_ptr_t blur(int size);

    void drawCircleColor(int x, int y, int radius, uint8_t color[4], uint8_t bgcolor[4], bool clear);
    void drawCircle(int x, int y, int radius, bool clear);
    void fillWithMask(int x, int y, RawImage *mask);

 private:
    unsigned char *buffer_;
    size_t buffer_len_;
    uint32_t w_;
    uint32_t h_;
    image_format_t format_;
    size_t version_;
};

image_ptr_t rawImageFromPyDict(py11::dict &image);
image_ptr_t newImage(uint32_t w, uint32_t h, bool enable_alpha);

}  // namespace py

typedef dexpert::py::image_ptr_t image_ptr_t;
typedef dexpert::py::RawImage RawImage;

}  // namespace dexpert



#endif  // SRC_PYTHON_RAW_IMAGE_H_
