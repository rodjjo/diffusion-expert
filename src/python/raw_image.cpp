#include <string>
#include <exception>

#include <CImg.h>

#include "src/python/raw_image.h"

using namespace cimg_library;

namespace dexpert {
namespace py {

namespace {
    const int format_channels[img_format_count] = {
        1,  // img_gray_8bit
        3,  // img_rgb
        4  // img_rgba
    };

    uint8_t white_color_rgba[4] {
        255, 255, 255, 255
    };

    uint8_t black_color_rgba[4] {
        0, 0, 0, 255
    };
    
    uint8_t no_color_rgba[4] {
        0, 0, 0, 0
    };
}  // unnamed namespace

RawImage::RawImage(const unsigned char *buffer, uint32_t w, uint32_t h, image_format_t format, bool fill_transparent) {
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
        if (format_ == img_rgba && fill_transparent) {
            memset(buffer_, 0, buffer_len_);
        } else {
            memset(buffer_, 255, buffer_len_);
        }
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

void RawImage::toPyDict(py11::dict &image) {
    std::string img_type;
    switch (format_)
    {
    case img_gray_8bit:
        img_type = "L";
        break;
    case img_rgb:
        img_type = "RGB";
        break;
    case img_rgba: 
        img_type = "RGBA";
        break;
    default:
        throw new std::string("Invalid format!");
        break;
    }
    image["width"] = w_;
    image["height"] = h_;
    image["mode"] = img_type;
    image["data"] = py11::bytes((const char *)buffer_, buffer_len_);
}

size_t RawImage::getVersion() {
    return version_;
}

void RawImage::incVersion() {
    ++version_;
}

image_ptr_t RawImage::duplicate() {
    return std::make_shared<RawImage>(
        buffer_, w_, h_, format_
    );
}

image_ptr_t RawImage::removeBackground(bool white) {
    image_ptr_t r;
    r.reset(new RawImage(NULL, w_, h_, img_rgba, false));
    int src_channels = format_channels[format_];
  
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

image_ptr_t RawImage::removeAlpha() {
    image_ptr_t r;

    r.reset(new RawImage(NULL, w_, h_, img_rgb));

    int src_channels = format_channels[format_];
   
    CImg<unsigned char> src(buffer_, src_channels, w_, h_, 1, true);
    CImg<unsigned char> img(r->buffer_, 3, w_, h_, 1, true);
    CImg<unsigned char> tmp(src, false);

    tmp.permute_axes("yzcx");
    img.permute_axes("yzcx");
    if (src_channels == 4) {
        tmp.fill("if(i3==0,255,i)", true); // remove the transparency
    }
    img.draw_image(0, 0, tmp); 
    // replace white per black and black per white
    img.fill("if((i0==i1&&i0==i2)&&(i0==255||i0==0),if(i0==0,255,0),i)", true);
    img.permute_axes("cxyz");

    return r;
}

void RawImage::drawCircleColor(int x, int y, int radius, uint8_t color[4], uint8_t bgcolor[4], bool clear) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= w_) x = w_ - 1;
    if (y >= h_) y = h_ - 1;
    int src_channels = format_channels[format_];
    CImg<unsigned char> img(buffer_, src_channels, w_, h_, 1, true);
    img.permute_axes("yzcx");
    if (clear) {
        img.draw_circle(x, y, radius, bgcolor);
    } else {
        img.draw_circle(x, y, radius, color);
    }
    img.permute_axes("cxyz");
    incVersion();
}

void RawImage::drawCircle(int x, int y, int radius, bool clear) {
    if (clear) {
        if (format_ == img_rgba)
            drawCircleColor(x, y, radius, black_color_rgba, no_color_rgba, true);
        else
            drawCircleColor(x, y, radius, black_color_rgba, white_color_rgba, true);
    } else {
        drawCircleColor(x, y, radius, black_color_rgba, white_color_rgba, false);
    }
}

void RawImage::fillWithMask(int x, int y, RawImage *mask) {
    auto image = this->duplicate();
    auto same_mask = mask->resizeCanvas(image->w(), image->h());
    CImg<unsigned char> msk(same_mask->buffer(), format_channels[same_mask->format()], same_mask->w(), same_mask->h(), 1, true);
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    src.permute_axes("yzcx");   
    msk.permute_axes("yzcx");
    msk.fill("if(i0==255&&i1==255&&i2==255,255,0)", true);
    src.draw_fill(x, y, black_color_rgba);
    msk.permute_axes("cxyz");
    src.permute_axes("cxyz");
    pasteAt(0, 0, same_mask.get(), image.get());
}

 void RawImage::pasteFill(RawImage *image) {
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    CImg<unsigned char> img(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    img.draw_image(0, 0, src.get_resize(image->w(), image->h()));
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::pasteAt(int x, int y, RawImage *image) {
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    CImg<unsigned char> img(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    img.draw_image(x, y, src);
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::pasteAt(int x, int y, RawImage *mask, RawImage *image) {
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    CImg<unsigned char> msk(mask->buffer(), format_channels[mask->format()], mask->w(), mask->h(), 1, true);
    CImg<unsigned char> img(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    msk.permute_axes("yzcx");
    img.draw_image(x, y, 0, 0, src, msk, 1, 255);
    msk.permute_axes("cxyz");
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::pasteAt(int x, int y, int w, int h, RawImage *image) {
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    CImg<unsigned char> img(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    src.permute_axes("yzcx");
    img.permute_axes("yzcx");
    img.draw_image(x, y, src.get_resize(w, h));
    img.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

void RawImage::pasteFrom(int x, int y, float zoom, RawImage *image) {
    int w = this->w();
    int h = this->h();
    if (zoom < 0.001) {
        zoom = 0.001;
    }

    w /= zoom;
    h /= zoom;

    // keep the area inside the source image    
    if (w <= 0 || h <= 0 || x >= image->w() || y >= image->h())  {
        return;
    }
    if (x + w > image->w()) {
        w = image->w() - x;
    }
    if (y + h > image->h()) {
        h = image->h() - y;
    }
    memset(this->buffer_, 255, this->buffer_len_); // turn this image white
    if (h < 0 || w < 0) {
        return;
    }
    CImg<unsigned char> src(image->buffer(), format_channels[image->format()], image->w(), image->h(), 1, true);
    CImg<unsigned char> self(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);

    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    float ratio, invert_w, invert_h;

    if (w > h) {
        ratio = h / (float)w;
        invert_w = w * zoom;
        invert_h = invert_w * ratio;
    } else {
        ratio = w / (float)h;
        invert_h = h * zoom;
        invert_w = invert_h * ratio;
    }
    self.draw_image(0, 0, src.get_crop(x, y, x + w, y + h).get_resize(invert_w, invert_h));
    if (invert_w < this->w()) {
        self.draw_rectangle(invert_w, 0, this->w(), this->h(), no_color_rgba);
    }
    if (invert_h < this->h()) {
        self.draw_rectangle(0, invert_h, this->w(), this->h(), no_color_rgba);
    }
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
}

image_ptr_t RawImage::resizeCanvas(uint32_t x, uint32_t y) {
    image_ptr_t result(new RawImage(NULL, x, y, this->format(), false));
    CImg<unsigned char> src(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    CImg<unsigned char> self(result->buffer(), format_channels[result->format()], result->w(), result->h(), 1, true);
    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    self.draw_image(0, 0, src);
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::resizeImage(uint32_t x, uint32_t y) {
    image_ptr_t result(new RawImage(NULL, x, y, this->format(), false));
    CImg<unsigned char> src(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    CImg<unsigned char> self(result->buffer(), format_channels[result->format()], result->w(), result->h(), 1, true);
    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    self.draw_image(0, 0, src.get_resize(x, y));
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::getCrop(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    image_ptr_t result(new RawImage(NULL, w, h, this->format(), false));
    CImg<unsigned char> src(this->buffer(), format_channels[this->format()], this->w(), this->h(), 1, true);
    CImg<unsigned char> self(result->buffer(), format_channels[result->format()], result->w(), result->h(), 1, true);
    src.permute_axes("yzcx");
    self.permute_axes("yzcx");
    self.draw_image(0, 0, src.get_crop(x, y, x + w, y + h));
    self.permute_axes("cxyz");
    src.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::blur(int size) {
    image_ptr_t result = this->duplicate();
    CImg<unsigned char> self(result->buffer(), format_channels[result->format()], result->w(), result->h(), 1, true);
    self.permute_axes("yzcx");
    self.blur(size, size, 0.0, 1, true);
    self.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::erode(int size) {
    image_ptr_t result = this->duplicate();
    CImg<unsigned char> self(result->buffer(), format_channels[result->format()], result->w(), result->h(), 1, true);
    self.permute_axes("yzcx");
    self.erode(size);
    self.permute_axes("cxyz");
    return result;
}

image_ptr_t RawImage::ensureMultipleOf8() {
    int diff_w = this->w() % 8;
    int diff_h = this->h() % 8;

    if (diff_w > 0 || diff_h > 0) {
        if (diff_w > 0) diff_w = 8 - diff_w;
        if (diff_h > 0) diff_h = 8 - diff_h;
        auto result = this->resizeImage(this->w() + diff_w, this->h() + diff_h);
        result->pasteAt(0, 0, this);
        printf("Image resized from %dx%d to %dx%d\n", this->w(), this->h(), result->w(), result->h());
        return result;
    } 

    return duplicate();
}

image_ptr_t RawImage::resizeLeft(int value) {
    auto img = std::make_shared<RawImage>(
        (const unsigned char *) NULL, this->w() + value, this->h(), img_rgba, false
    );
    img->pasteAt(value, 0, this);
    return img;
}

image_ptr_t RawImage::resizeRight(int value) {
    return resizeCanvas(this->w() + value, this->h());
}

image_ptr_t RawImage::resizeTop(int value) {
    auto img = std::make_shared<RawImage>(
        (const unsigned char *) NULL, this->w(), this->h() + value, img_rgba, false
    );
    img->pasteAt(0, value, this);
    return img;
}

image_ptr_t RawImage::resizeBottom(int value) {
    return resizeCanvas(this->w(), this->h() + value);
}

image_ptr_t rawImageFromPyDict(py11::dict &image) {
    if (!image.contains("data")) {
        return image_ptr_t();
    }
    auto img_mode = image["mode"].cast<std::string>();
    auto format = img_gray_8bit;
    if (img_mode == "RGB")
        format = img_rgb;
    else if (img_mode == "RGBA")
        format = img_rgba;
    const auto &data = image["data"].cast<std::string>();
    return std::make_shared<RawImage>(
        (const unsigned char *)&data[0],
        image["width"].cast<py11::int_>(),
        image["height"].cast<py11::int_>(),
        format
    );
}

image_ptr_t newImage(uint32_t w, uint32_t h, bool enable_alpha) {
    return std::make_shared<RawImage>(
        (const unsigned char *) NULL, w, h, enable_alpha ? img_rgba : img_rgb
    );
}


} // namespace py
} // namespace dexpert
