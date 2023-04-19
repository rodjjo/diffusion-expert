#include <string>
#include <exception>

#include <CImg.h>

#include "src/python/raw_image.h"
#include "src/python/guard.h"

using namespace cimg_library;

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
    if (buffer) {
        memcpy(buffer_, buffer, buffer_len_);
    } else {
        memset(buffer_, 255, buffer_len_);
    }
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

std::shared_ptr<RawImage> RawImage::removeBackground(bool white) {
    std::shared_ptr<RawImage> r;
    r.reset(new RawImage(NULL, w_, h_, img_rgba));
    int src_channels = 1;
    if (format_ == img_rgb) {
        src_channels = 3;
    } else if (format_ == img_rgba) {
        src_channels = 4;
    }
   
    CImg<unsigned char> src(buffer_, src_channels, w_, h_, 1, true);
    CImg<unsigned char> img(r->buffer_, 4, w_, h_, 1, true);
    
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    img.draw_image(0, 0, src);
    
    if (white) {
        img.fill("if(i0==255&&i1==255&&i2==255,0,i)", true);
    } else {
        img.fill("if(i0!=0||i1!=0||i2!=0,i,0)", true);
    }
   
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");

    return r;
}

std::shared_ptr<RawImage> RawImage::removeAlpha() {
    std::shared_ptr<RawImage> r;

    r.reset(new RawImage(NULL, w_, h_, img_rgb));

    int src_channels = 1;
    if (format_ == img_rgb) {
        src_channels = 3;
    } else if (format_ == img_rgba) {
        src_channels = 4;
    }
   
    CImg<unsigned char> src(buffer_, src_channels, w_, h_, 1, true);
    CImg<unsigned char> img(r->buffer_, 3, w_, h_, 1, true);
    CImg<unsigned char> tmp(src, false);

    tmp.permute_axes("yzcx");
    img.permute_axes("yzcx");
    tmp.fill("if(i3==0,255,i)", true); // remove the transparency
    img.draw_image(0, 0, tmp); 
    // replace white per black and black per white
    img.fill("if((i0==i1&&i0==i2)&&(i0==255||i0==0),if(i0==0,255,0),i)", true);
    img.permute_axes("cxyz");

    return r;
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

image_ptr_t newImage(uint32_t w, uint32_t h, bool enable_alpha) {
    return std::make_shared<RawImage>(
        (const unsigned char *) NULL, w, h, enable_alpha ? img_rgba : img_rgb
    );
}



} // namespace py
} // namespace dexpert
