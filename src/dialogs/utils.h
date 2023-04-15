/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_DIALOGS_UTILS_H_
#define SRC_DIALOGS_UTILS_H_

#include "src/python/raw_image.h"

namespace dexpert
{


image_ptr_t open_image_from_dialog();
bool save_image_with_dialog(image_ptr_t img);


} // namespace dexpert


#endif  // SRC_DIALOGS_UTILS_H_