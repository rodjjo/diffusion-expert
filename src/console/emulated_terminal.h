/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_CONSOLE_EMULATED_TERMINAL_H_
#define SRC_CONSOLE_EMULATED_TERMINAL_H_

#include <mutex>
#include <thread>
#include <memory>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace dexpert {

typedef enum {
    log_source_stdout,
    log_source_stderr
} log_source_t;

class EmulatedTerminal;

std::shared_ptr<EmulatedTerminal> get_terminal(log_source_t src_type);

class EmulatedTerminal {
    friend std::shared_ptr<EmulatedTerminal> get_terminal(log_source_t);
    EmulatedTerminal(log_source_t src_type_);

 public:
    ~EmulatedTerminal();
    EmulatedTerminal (const EmulatedTerminal &) = delete;
    EmulatedTerminal & operator = (const EmulatedTerminal &) = delete;
    void draw(Fl_Widget *target, int font_face, int font_size, int font_height);
    int version();
    void copy_to_clipboard();

 private:
    bool init_console();
    void run();
    void initialize();
    void append(const char *data, size_t len);
    const unsigned char *vt100_Escape(const unsigned char *sz, int cnt);
    void next_line();
    void buff_clear(int offset, int len);
    const unsigned char *telnet_options(const unsigned char *p, int cnt);
    void check_cursor_y();

 private:
    std::unique_ptr<std::thread> thread_;
    std::mutex mtx_;
    log_source_t src_type = log_source_stdout;
    int pipefd[2] = { 0, };
    

 private:
    char c_attr = 7;
    char save_attr = 7;
    char *buff = NULL;
    char *attr = NULL;
    int *line = NULL;
    bool terminated_ = false;
    int cursor_x = 0;
    int cursor_y = 0;
    int save_x = 0;
    int save_y = 0;
    int size_x = 80;
    int size_y = 24;
    int screen_x = 0;
    int screen_y = 0;
    int roll_top = 0;
    int roll_bot = 24-1;
    int recv0 = 0;

    int version_ = 0;

    bool bEscape = false; // escape sequence processing mode
    int ESC_idx = 0;      // current index for ESC_code
    char ESC_code[32];    // cumulating the current escape sequence before process
    char tabstops[256];

    bool bTitle = false;
    bool bInsert = false;     // insert mode, for inline editing for commands
    bool bGraphic = false;    // graphic character mode, for text mode drawing
    bool bCursor = false;     // display cursor or not
    bool bAppCursor = false;  // app cursor mode for vi
    bool bAltScreen = false;  // alternative screen for vi
    bool bScrollbar = false;  // show scrollbar when true
    bool bDragSelect = false; // mouse dragged to select text, instead of scroll text
    bool bBracket = false;    // bracketed paste mode
    bool bWraparound = false;
    bool bOriginMode = false;
};

}  // namespace dexpert 

#endif  // SRC_CONSOLE_EMULATED_TERMINAL_H_
