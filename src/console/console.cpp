#include "src/console/console.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace dexpert
{
    Console::Console(int X, int Y, int W, int H, std::shared_ptr<EmulatedTerminal> terminal) : Fl_Widget(X, Y, W, H, "")
    {
        version_ = 0;
        terminal_ = terminal;
        textfont(FL_COURIER);
        textsize(16);
        Fl::add_timeout(0.01, Console::validate, this);
        box(FL_FLAT_BOX);
        color(FL_BLACK);
    }

    Console::~Console()
    {
        Fl::remove_timeout(Console::validate, this);
    }

    void Console::validate(void *ud) {
        static bool console_closed = true;
        if (!console_closed) {
            HWND hconsole = GetConsoleWindow();
            if (hconsole != NULL) {
                DWORD pid = 0;
                GetWindowThreadProcessId(hconsole, &pid);
                if (pid == GetCurrentProcessId()) {
                    ShowWindow(hconsole, SW_HIDE);
                }
                console_closed = true;
            }
        }
        Fl::repeat_timeout(0.33, Console::validate, ud); // retrigger timeout
        Console *c = (Console *)ud;
        if (c->version_ != c->terminal_->version()) {
            c->redraw();
        }
    }

    void Console::draw()
    {
        version_ = terminal_->version();
        terminal_->draw(this, getFontFace(), getFontSize(), getFontHeight());
    };

    void Console::textfont(Fl_Font fontface)
    {
        font_face = fontface;
        fl_font(font_face, font_size);
        font_width = fl_width("abcdefghij") / 10;
        font_height = fl_height();
    }

    void Console::textsize(int fontsize)
    {
        font_size = fontsize;
        fl_font(font_face, font_size);
        font_width = fl_width("abcdefghij") / 10;
        font_height = fl_height();
    }

    int Console::getFontWidth()
    {
        return font_width;
    }

    int Console::getFontFace()
    {
        return font_face;
    }

    int Console::getFontSize()
    {
        return font_size;
    }

    int Console::getFontHeight()
    {
        return font_height;
    }

} // namespace dexpert
