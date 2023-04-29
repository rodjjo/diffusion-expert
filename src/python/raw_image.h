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

    std::shared_ptr<RawImage> duplicate();
    std::shared_ptr<RawImage> removeBackground(bool white);
    std::shared_ptr<RawImage> removeAlpha();
    std::shared_ptr<RawImage> resizeCanvas(uint32_t x, uint32_t y);
    std::shared_ptr<RawImage> resizeImage(uint32_t x, uint32_t y);
    void drawCircle(int x, int y, int radius, bool clear);

 private:
    unsigned char *buffer_;
    size_t buffer_len_;
    uint32_t w_;
    uint32_t h_;
    image_format_t format_;
    size_t version_;
};

typedef std::shared_ptr<RawImage> image_ptr_t;

image_ptr_t rawImageFromPyDict(py11::dict &image);
image_ptr_t newImage(uint32_t w, uint32_t h, bool enable_alpha);

}  // namespace py

typedef dexpert::py::image_ptr_t image_ptr_t;
typedef dexpert::py::RawImage RawImage;

}  // namespace dexpert



#endif  // SRC_PYTHON_RAW_IMAGE_H_
