#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "misc/utils.h"


namespace dfe
{
namespace 
{
    std::wstring executable_dir;
    std::wstring python_exe_path;
    std::wstring pysources_dir;
} 

const std::wstring& executableDir() {
    if (executable_dir.empty()) {
#ifdef _WIN32
    wchar_t path[1024] = { 0, };
    if (GetModuleFileNameW(NULL, path, (sizeof(path) / sizeof(wchar_t)) -1) != 0) {
        executable_dir = path;
    }
#else
    //TODO: create linux implementation
#endif
        size_t latest = executable_dir.find_last_of(L"/\\");
        if (latest != std::wstring::npos) {
            executable_dir = executable_dir.substr(0, latest);
        } else {
            executable_dir = std::wstring();
        }
    }

    return executable_dir;
}


const std::wstring& pythonExecutablePath() {
    if (python_exe_path.empty()) {
        python_exe_path = executableDir() + L"/python.exe";
    }
    return python_exe_path;
}

const std::wstring& sourcesDirectory() {
    if (pysources_dir.empty()) {
        pysources_dir = executableDir() + L"/../diffusion_expert";
    }
    return pysources_dir;
}

std::string filepath_dir(const std::string & path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(0, pos + 1);
    }
    return path;
}

void blur_gl_contents(int w, int h, int mouse_x, int mouse_y) {
    if (false) { // todo(RODRIGO): disable blur from config
        return;
    }

    const int half_area = Fl::event_shift() != 0 ? 80 : 40;
    float fx = 2.0 / w;
    float fy = 2.0 / h;
    float hx = half_area * fx;
    float hy = half_area * fy;
    float x1 = (mouse_x * fx - hx) - 1.0;
    float y1 = 1.0 - (mouse_y * fy - hy);
    float x2 = (mouse_x * fx + hx) - 1.0;
    float y2 = 1.0 - (mouse_y * fy + hy);

    glColor4f(0.2, 0.2, 0.2, 1.0);
    glBegin(GL_QUADS);

    // left
    glVertex2f(-1.0, 1.0);
    glVertex2f(x1, 1.0);
    glVertex2f(x1, -1.0);
    glVertex2f(-1.0, -1.0);
    
    // right
    glVertex2f(1.0, 1.0);
    glVertex2f(x2, 1.0);
    glVertex2f(x2, -1.0);
    glVertex2f(1.0, -1.0);

    // center top
    glVertex2f(x1, y1);
    glVertex2f(x1, 1.0);
    glVertex2f(x2, 1.0);
    glVertex2f(x2, y1);

    // center bottom
    glVertex2f(x1, y2);
    glVertex2f(x2, y2);
    glVertex2f(x2, -1.0);
    glVertex2f(x1, -1.0);

    glEnd();
}

#ifdef _WIN32

typedef struct {
    uintptr_t result;
    char title_buffer[1024];
    const char *title;
} enumeration_param_t;

BOOL enumerationProc(
  _In_ HWND   hwnd,
  _In_ LPARAM lParam
) {
    enumeration_param_t *r = (enumeration_param_t *)lParam;
    r->title_buffer[sizeof(r->title_buffer) - 1] = '\0';
    GetWindowTextA(hwnd, r->title_buffer, sizeof(r->title_buffer) - 1);
    if (strcmp(r->title, r->title_buffer) == 0) {
        r->result = (uintptr_t)hwnd;
        return FALSE;
    }
    return TRUE;
}

uintptr_t find_current_thread_window(const char *title, uintptr_t parent_hwnd) {
    enumeration_param_t result;
    result.result = (uintptr_t) 0;
    result.title = title;
    if (parent_hwnd) {
        EnumChildWindows((HWND)parent_hwnd, &enumerationProc, (LPARAM)&result);
    } else {
        EnumThreadWindows(GetCurrentThreadId(), &enumerationProc, (LPARAM)&result);
    }
    return result.result;
}
#endif

} // namespace dfe