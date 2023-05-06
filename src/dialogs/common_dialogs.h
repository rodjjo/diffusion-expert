/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_DIALOGS_COMMON_DIALOGS_H_
#define SRC_DIALOGS_COMMON_DIALOGS_H_

#include <string>

namespace dexpert {

typedef enum {
    r_yes = 0,
    r_no,
    r_cancel
} response_t;

bool ask(const char *message);
response_t yes_nc(const char *message);
void show_error(const char *message);
const char *ask_value(const char *message);


std::string choose_image_to_open(std::string* current_dir);
std::string choose_image_to_save(std::string* current_dir);

bool pickup_color(const char* title, uint8_t *r, uint8_t *g, uint8_t *b);

}  // namespace dexpert

#endif  // SRC_DIALOGS_COMMON_DIALOGS_H_