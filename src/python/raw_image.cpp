#include <string>
#include <exception>
#include "src/python/raw_image.h"
#include "src/python/guard.h"

namespace dexpert {
namespace py {

RawImage::RawImage(const unsigned char *buffer, uint32_t w, uint32_t h, image_format_t format) {
    format_ = format;
    buffer_len_ = w * h;
    w_ = w;
    h_ = h;
    switch (format_) {
        case img_rgb:
            buffer_len_ *= 3;
            break;
        case img_rgba:
            buffer_len_ *= 4;
            break;
    }
    buffer_ = (unsigned char *)malloc(buffer_len_);
    memcpy(buffer_, buffer, buffer_len_);
    version_ = (size_t) buffer_; // randomize the version
}

RawImage::~RawImage() {
    free(buffer_);
}

const unsigned char *RawImage::buffer() {
    return buffer_;
}

image_format_t RawImage::format() {
    return format_;
}

uint32_t RawImage::h() {
    return h_;
}

uint32_t RawImage::w() {
    return w_;
}

void RawImage::toPyDict(PyObject *dict) {
    std::wstring img_type;
    switch (format_)
    {
    case img_gray_8bit:
        img_type = L"L";
        break;
    case img_rgb:
        img_type = L"RGB";
        break;
    case img_rgba: 
        img_type = L"RGBA";
        break;
    default:
        throw new std::string("Invalid format!");
        break;
    }

    ObjGuard guard;
    PyObject *po_buffer = guard(PyBytes_FromStringAndSize((const char *)buffer_, buffer_len_));
    PyObject *po_w = guard(PyLong_FromSize_t(w_));
    PyObject *po_h = guard(PyLong_FromSize_t(h_));
    PyObject* po_mode = guard(PyUnicode_FromWideChar(img_type.c_str(), -1));

    PyDict_SetItemString(dict, "width", po_w);
    PyDict_SetItemString(dict, "height", po_h);
    PyDict_SetItemString(dict, "mode", po_mode);
    PyDict_SetItemString(dict, "data", po_buffer);
}

image_ptr_t rawImageFromPyDict(PyObject * dict) {
    image_ptr_t r;

    // when we get item from dict whe do not inc the ref counter
    PyObject *po_buffer = PyDict_GetItemString(dict, "data");
    PyObject *po_w = PyDict_GetItemString(dict, "width");
    PyObject *po_h = PyDict_GetItemString(dict, "height");
    PyObject *po_mode = PyDict_GetItemString(dict, "mode");

    bool valid = po_buffer != NULL && po_w != NULL && po_h != NULL && po_mode != NULL;

    image_format_t format = img_rgba;

    if (valid) {
        valid = (PyLong_Check(po_w) != 0) && (PyLong_Check(po_h) != 0);
        valid = valid && (PyUnicode_Check(po_mode) != 0) && (PyBytes_Check(po_buffer) != 0);
        Py_ssize_t size;
        wchar_t *mode = PyUnicode_AsWideCharString(po_mode, &size);
        std::wstring img_mode(mode, size);
        PyMem_Free(mode);
        if (img_mode == L"L")
            format = img_gray_8bit;
        else if (img_mode == L"RGB")
            format = img_rgb;
        else if (img_mode == L"RGBA")
            format = img_rgba;
        else
            valid = false;
    }

    if (valid) {
        r.reset(new RawImage(
            (const unsigned char*)PyBytes_AsString(po_buffer),
            PyLong_AsSize_t(po_w),
            PyLong_AsSize_t(po_h),
            format
        ));
    }

    return r;
}

size_t RawImage::getVersion() {
    return version_;
}

void RawImage::incVersion() {
    ++version_;
}

std::shared_ptr<RawImage> RawImage::duplicate() {
    return std::make_shared<RawImage>(
        buffer_, w_, h_, format_
    );
}


} // namespace py
} // namespace dexpert
