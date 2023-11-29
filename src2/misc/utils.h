#pragma once

#include <string>

namespace dfe {

const std::wstring& executableDir();
const std::wstring& pythonExecutablePath();
const std::wstring& sourcesDirectory();
std::string filepath_dir(const std::string & path);
void blur_gl_contents(int w, int h, int mouse_x, int mouse_y);
#ifdef _WIN32
uintptr_t find_current_thread_window(const char *title, uintptr_t parent_hwnd = (uintptr_t)0);
#endif
bool rectRect(int r1x, int r1y, int r1w, int r1h, int r2x, int r2y, int r2w, int r2h);
}