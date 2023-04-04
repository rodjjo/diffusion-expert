/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_CONSOLE_CONSOLE_H_
#define SRC_CONSOLE_CONSOLE_H_

#include <memory>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include "src/console/emulated_terminal.h"

namespace dexpert {

class Console: public Fl_Widget { 
 public:
    Console(int X,int Y,int W,int H, std::shared_ptr<EmulatedTerminal> terminal);
    virtual ~Console();

    void textfont(Fl_Font fontface);
    void textsize(int fontsize);
    int getFontWidth();
    int getFontFace();
    int getFontSize();
    int getFontHeight();

 protected:
    void draw() override;

 private:
    static void validate(void *ud);

 private:
    int version_;
    std::shared_ptr<EmulatedTerminal> terminal_;
    int font_width = 10;
    int font_height = 16;
    int font_size;		//current font size, should equal to height
    int font_face;		//current font face
    
};

}  // namespace dexpert 

#endif  // SRC_CONSOLE_CONSOLE_H_
