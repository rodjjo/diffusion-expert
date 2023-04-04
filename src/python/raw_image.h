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

 private:
    unsigned char *buffer_;
    size_t buffer_len_;
    uint32_t w_;
    uint32_t h_;
    image_format_t format_;
};

std::shared_ptr<RawImage> rawImageFromPyDict(PyObject * dict);

}  // namespace py
}  // namespace dexpert

#endif  // SRC_PYTHON_RAW_IMAGE_H_
