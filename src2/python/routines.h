#pragma once

#include <pybind11/embed.h> 
#include <functional>

#include "python/code.h"
#include "python/image.h"

namespace py11 = pybind11;

namespace dfe
{
namespace py
{

image_ptr_t open_image(const char* path);
py11::dict load_config();
void store_config(const py11::dict& config);
image_ptr_t remove_background(RawImage* img, const py11::dict& params);
    
} // namespace py
} // namespace dfe

