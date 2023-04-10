/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PYTHON_RAW_IMAGE_H_
#define SRC_PYTHON_RAW_IMAGE_H_

#include <memory>
#include <Python.h>

namespace dexpert {
namespace py {

typedef enum {
    img_gray_8bit,
    img_rgb,
    img_rgba
} image_format_t;


class RawImage {
 public:
    RawImage(const unsigned char *buffer, uint32_t w, uint32_t h, image_format_t format);
    virtual ~RawImage();
    void toPyDict(PyObject *dict);
    const unsigned char *buffer();
    image_format_t format();
    uint32_t h();
    uint32_t w();
    size_t getVersion();
    void incVersion();
    void setVariation(int value);
    int getVariation();

 private:
    int variation_ = 0;
    unsigned char *buffer_;
    size_t buffer_len_;
    uint32_t w_;
    uint32_t h_;
    image_format_t format_;
    size_t version_;
};

typedef std::shared_ptr<RawImage> image_ptr_t;

image_ptr_t rawImageFromPyDict(PyObject * dict);
}  // namespace py

typedef dexpert::py::image_ptr_t image_ptr_t;
typedef dexpert::py::RawImage RawImage;

}  // namespace dexpert



#endif  // SRC_PYTHON_RAW_IMAGE_H_
