#pragma once

#include <Fl/Fl_Simple_Terminal.H>
#include "terminal/terminal.h"

namespace dfe
{

class Console : public Fl_Simple_Terminal, protected Terminal {
  public:
    Console(int X, int Y, int W, int H, const char *l = NULL);
    virtual ~Console();
  protected:
    void append(const char *data) override;
// Fl__Simple__Terminal  
};


} // namespace dfe
