#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "src/python/wrapper.h"
#include "src/windows/main_window.h"

int main(int argc, char **argv)
{
    int result = 0;
    std::thread gui_thread([&result] {
        Fl::scheme("gtk+");

#ifdef _WIN32
        HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("Dexpert202310"));
        if (hMutex == NULL) {
            result = 1;
            return;
        }
        else if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(hMutex);
            result = 1;
            return;
        }
#endif

        auto w = new dexpert::MainWindow(); // fltk deletes the object after we run it.
        result = w->run();

#ifdef _WIN32
        CloseHandle(hMutex);
#endif
        dexpert::py::py_end();
    });

    dexpert::py::py_main();
    gui_thread.join();
    return result;
}

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif