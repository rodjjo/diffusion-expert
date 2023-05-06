#include <string>
#include <Windows.h>


const std::wstring& executableDir() {
    static std::wstring executableDir_;
    if (executableDir_.empty()) {
#ifdef _WIN32
    wchar_t path[1024] = { 0, };
    if (GetModuleFileNameW(NULL, path, (sizeof(path) / sizeof(wchar_t)) -1) != 0) {
        executableDir_ = path;
    }
#else
    //TODO: create linux implementation
#endif
        size_t latest = executableDir_.find_last_of(L"/\\");
        if (latest != std::wstring::npos) {
            executableDir_ = executableDir_.substr(0, latest);
        } else {
            executableDir_ = std::wstring();
        }
    }

    return executableDir_;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    /*
        To redirect python console in diffusion-exp.exe whe have to start the process with a console window.
        We can't AllocConsole there because the console handles should not change. It should have subsystem -mconsole compiler flag.
        So we have this process here that allocates a console window, hides it before starting diffusion-exp.exe (we dont want a console window showing there).
    */
    AllocConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    std::wstring where = executableDir() + L"/diffusion-exp.exe";
    PROCESS_INFORMATION info;
    STARTUPINFOW si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &info, sizeof(info) );
    CreateProcessW(NULL, (LPWSTR) where.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si , &info);
    return 0;
}