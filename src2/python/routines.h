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
    
} // namespace py
} // namespace dfe

