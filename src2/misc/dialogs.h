#pragma once

#include <string>

namespace dfe
{
    
typedef enum {
    r_yes = 0,
    r_no,
    r_cancel
} response_t;

bool ask(const char *message);
response_t yes_nc(const char *message);
void show_error(const char *message);

std::string choose_image_to_open_fl(std::string* current_dir);
std::string choose_image_to_save_fl(std::string* current_dir);
    
} // namespace dfe
