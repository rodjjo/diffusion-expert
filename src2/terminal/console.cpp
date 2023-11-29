#include "terminal/console.h"

namespace dfe
{

Console::Console(int X, int Y, int W, int H, const char *l) : Terminal(), Fl_Simple_Terminal(
        X, Y, W, H, l) {
}

Console::~Console() {
}

void Console::append(const char *data)  {
    this->printf("%s", data);
}
    
} // namespace dfe
