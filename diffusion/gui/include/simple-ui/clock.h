#pragma once

#include <inttypes.h>

namespace dfe_ui
{
namespace clock {
    void restart_clock();
    int64_t current_microseconds();
    int32_t current_miliseconds();    
    bool editor_cursor_visible();
} 
} // namespace dfe_ui
