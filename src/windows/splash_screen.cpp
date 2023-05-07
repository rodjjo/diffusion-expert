#include <FL/Fl.H>
#include "src/windows/splash_screen.h"
#include "src/python/wrapper.h"

namespace dexpert
{
    
SplashScreen::SplashScreen() : Fl_Window(200, 80, "Diffusion Expert") {
    this->begin();
    loading_label_ = new Fl_Box(0, 0, w(), h(), "Python is loading...");
    this->end();
    this->set_modal();
    this->show();
}

SplashScreen::~SplashScreen() {
}

void SplashScreen::open() {
    this->position(Fl::w() / 2 - this->w() / 2, Fl::h() / 2 - this->h() / 2);
    this->show();
}

void SplashScreen::close() {
    this->hide();
}

void wait_python() {
    static SplashScreen w;
    w.open();
    while(!dexpert::py::py_ready()) {
        if (!w.shown()) {
            w.show();
        }
        Fl::wait(0.03);
    }
    w.close();
}


} // namespace dexpert
