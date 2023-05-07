#ifndef SRC_WINDOWS_SPLASH_SCREEN_H
#define SRC_WINDOWS_SPLASH_SCREEN_H

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

namespace dexpert
{
    
class SplashScreen : public Fl_Window {
    public:
        SplashScreen();
        virtual ~SplashScreen();
        void open();
        void close();
    private:
        Fl_Box *loading_label_;
};

void wait_python();

} // namespace dexpert

#endif  // SRC_WINDOWS_SPLASH_SCREEN_H
