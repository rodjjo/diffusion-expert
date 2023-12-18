#pragma once

#include <stdint.h>
#include <list>
#include <vector>
#include <string>
#include <functional>
#include <pybind11/embed.h> 

#include "python/code.h"
#include "python/image.h"

namespace py11 = pybind11;

namespace dfe
{
namespace py
{

image_ptr_t open_image(const char* path);
void save_image(const char* path, image_ptr_t image);
py11::dict load_config();
void store_config(const py11::dict& config);
image_ptr_t remove_background(RawImage* img, const py11::dict& params);
std::vector<std::pair<bool, std::string> > list_models();
std::list<image_ptr_t> generate_image(py11::dict parameters);
std::vector<std::string> list_schedulers();

} // namespace py
} // namespace dfe

