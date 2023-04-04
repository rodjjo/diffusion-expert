#include <stdio.h>
#include <iostream>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

#include "src/console/emulated_terminal.h"
#include "src/python/wrapper.h"

#define OUT_BUFF_SIZE 512
#define LINE_SIZE 4096
#define LINE_COUNT 64
#define HALF_LINES 32
#define BUFFER_SIZE (LINE_SIZE * LINE_COUNT)

#ifdef _WIN32
#define DUP _dup
#define DUP2 _dup2
#define CLOSE _close
#define READ _read
#define WRITE _write
#define FILENO _fileno
#define MAKE_PIPE(X) _pipe((X), OUT_BUFF_SIZE, O_BINARY)
#define NULL_DEVICE "nul"
#else
#define DUP dup
#define DUP2 dup2
#define CLOSE close
#define READ read
#define WRITE write
#define FILENO fileno
#define MAKE_PIPE(X) _pipe((X), OUT_BUFF_SIZE, O_BINARY)
#define NULL_DEVICE "/dev/null"
#endif

#define STREAM(X) ((X) == log_source_stdout ? stdout: stderr)
#define FILENOS(X) (FILENO(STREAM(X)))


static void _logIt(const char *text)
{
    static FILE *fp = fopen("console.log", "w");
    fprintf(fp, "%s\n", text);
    fflush(fp);
}

// #define LOG(X) _logIt((X))
#define LOG(X)

namespace dexpert
{
    namespace
    {
        std::shared_ptr<EmulatedTerminal> terminals[2];
        int default_fn[2] = {1, 2};

        struct Startup
        {
            Startup()
            {
                default_fn[log_source_stdout] = 1;
                default_fn[log_source_stderr] = 2;
                get_terminal(log_source_stdout); // just to start the stdout capture
                get_terminal(log_source_stderr); // just to start the stderr capture
                dexpert::py::get_py(); // initialize python
            }
        };

        Startup initializer; // start the captures
    }

    std::shared_ptr<EmulatedTerminal> get_terminal(log_source_t src_type)
    {
        if (!terminals[src_type])
        {
            terminals[src_type].reset(new EmulatedTerminal(src_type));
        }
        return terminals[src_type];
    }

    EmulatedTerminal::EmulatedTerminal(log_source_t src_type_)
    {
        src_type = src_type_;
        if (init_console())
        {
            LOG("EmulatedTerminal 1.");
            initialize();
            thread_.reset(new std::thread([this]
                                          { run(); }));
        }
    }

    EmulatedTerminal::~EmulatedTerminal()
    {
        terminated_ = true;
        if (thread_)
        {
            fclose(STREAM(src_type));
            thread_->join();
        }
        free(buff);
        free(line);
        free(attr);
    }

    int EmulatedTerminal::version()
    {
        return version_;
    }

    void EmulatedTerminal::initialize()
    {
        buff = (char *)malloc(BUFFER_SIZE);
        attr = (char *)malloc(BUFFER_SIZE);
        line = (int *)malloc(LINE_SIZE * sizeof(int));

        memset(line, 0, LINE_SIZE * sizeof(int));
        memset(buff, 0, BUFFER_SIZE);
        memset(attr, 0, BUFFER_SIZE);

        cursor_y = cursor_x = 0;
        screen_y = 0;
        c_attr = 7; // default black background, white foreground
        recv0 = 0;
        ESC_idx = 0;
        bInsert = bEscape = bGraphic = false;
        bBracket = bAltScreen = bAppCursor = bOriginMode = false;
        bWraparound = true;
        bScrollbar = false;
        bCursor = true;

        memset(tabstops, 0, 256);
        for (int i = 0; i < 256; i += 8)
            tabstops[i] = 1;

        size_x = 80;
        size_y = 25;
        roll_top = 0;
        roll_bot = size_y - 1;
    }

    bool EmulatedTerminal::init_console()
    {
        LOG("CONSOLE INITIALIZE.");

        const char *stream_name = src_type == log_source_stdout ? "stdout" : "stderr";

        if (MAKE_PIPE(pipefd) == -1)
        {
            LOG("err CONSOLE INITIALIZE 2.");

            printf("Could not redirect %s. Create pipe failed\n", stream_name);
            return false;
        }

        // intptr_t h = (intptr_t) (src_type == log_source_stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
        // int fd = _open_osfhandle((h), O_TEXT);
        
        if (DUP2(pipefd[1], FILENOS(src_type)) == -1) {
            LOG("CONSOLE INITIALIZE -4.");    
        }

        setvbuf(STREAM(src_type), NULL, _IONBF, 0);

        LOG("CONSOLE INITIALIZE 4.");
        return true;
    }

    void EmulatedTerminal::run()
    {
        LOG("EmulatedTerminal::run() 1.");

        char temp[LINE_SIZE] = {
            0,
        };

        int count = 0;
        int readed = 0;
        while (!terminated_)
        {
            readed = READ(pipefd[0], temp, sizeof(temp));
            if (terminated_)
            {
                break;
            }
            if (readed > 0)
            {
                append(temp, readed);
            }
        }

        LOG("EmulatedTerminal::run() 2.");

        CLOSE(pipefd[0]);
        CLOSE(pipefd[1]);
    }

#define TNO_IAC 0xff
#define TNO_DONT 0xfe
#define TNO_DO 0xfd
#define TNO_WONT 0xfc
#define TNO_WILL 0xfb
#define TNO_SUB 0xfa
#define TNO_SUBEND 0xf0
#define TNO_ECHO 0x01
#define TNO_AHEAD 0x03
#define TNO_STATUS 0x05
#define TNO_LOGOUT 0x12
#define TNO_WNDSIZE 0x1f
#define TNO_TERMTYPE 0x18
#define TNO_NEWENV 0x27

    unsigned char TERMTYPE[] = { // vt100
        0xff, 0xfa, 0x18, 0x00, 0x76, 0x74, 0x31, 0x30, 0x30, 0xff, 0xf0};

    const unsigned char *EmulatedTerminal::telnet_options(const unsigned char *p, int cnt)
    {
        const unsigned char *q = p + cnt;
        while (*p == 0xff && p < q)
        {
            unsigned char negoreq[] = {0xff, 0, 0, 0, 0xff, 0xf0};
            switch (p[1])
            {
            case TNO_WONT:
            case TNO_DONT:
                p += 3;
                break;
            case TNO_DO:
                p += 3;
                break;
            case TNO_WILL:
                p += 3;
                break;
            case TNO_SUB:
                while (*p != 0xff && p < q)
                    p++;
                break;
            case TNO_SUBEND:
                p += 2;
            }
        }
        return p + 1;
    }

    void EmulatedTerminal::append(const char *data, size_t len)
    {
        const unsigned char *p = (const unsigned char *)data;
        const unsigned char *zz = p + len;

        std::unique_lock<std::mutex> lk(mtx_);
        ++version_;

        if (bEscape)
            p = vt100_Escape(p, zz - p);

        while (p < zz)
        {
            unsigned char c = *p++;

            if (bTitle)
            {
                if (c == 0x07)
                {
                    bTitle = false;
                }
                continue;
            }

            switch (c)
            {
            case 0x00:
            case 0x0e:
            case 0x0f:
                break;
            case 0x07:
                // fl_beep(FL_BEEP_DEFAULT);
                break;
            case 0x08:
                if (cursor_x > line[cursor_y])
                {
                    if ((buff[cursor_x--] & 0xc0) == 0x80) // utf8 continuation byte
                        while ((buff[cursor_x] & 0xc0) == 0x80)
                            cursor_x--;
                }
                break;
            case 0x09:
            {
                int l;
                do
                {
                    attr[cursor_x] = c_attr;
                    buff[cursor_x++] = ' ';
                    l = cursor_x - line[cursor_y];
                } while (l <= size_x && tabstops[l] == 0);
            }
            break;
            case 0x0a:
            case 0x0b:
            case 0x0c:
                if (bAltScreen || line[cursor_y + 2] != 0)
                { // IND to next line
                    vt100_Escape((unsigned char *)"D", 1);
                }
                else
                { // LF and newline
                    cursor_x = line[cursor_y + 1];
                    attr[cursor_x] = c_attr;
                    buff[cursor_x++] = 0x0a;
                    next_line();
                }
                break;
            case 0x0d:
                if (cursor_x - line[cursor_y] == size_x + 1 && *p != 0x0a)
                    next_line(); // soft line feed
                else
                    cursor_x = line[cursor_y];
                break;
            case 0x1b:
                p = vt100_Escape(p, zz - p);
                break;
            case 0xff:
                p = telnet_options(p - 1, zz - p + 1);
                break;
            case 0xe2:
                if (bAltScreen)
                { // utf8 box drawing hack
                    c = ' ';
                    if (*p++ == 0x94)
                    {
                        switch (*p)
                        {
                        case 0x80:
                        case 0xac:
                        case 0xb4:
                        case 0xbc:
                            c = '_';
                            break;
                        case 0x82:
                        case 0x94:
                        case 0x98:
                        case 0x9c:
                        case 0xa4:
                            c = '|';
                            break;
                        }
                    }
                    p++;
                } // fall through
            default:
                if (bGraphic)
                {
                    switch (c)
                    { // charset 2 box drawing
                    case 'q':
                        c = '_';
                        break;
                    case 'x':
                        c = '|';
                    case 't':
                    case 'u':
                    case 'm':
                    case 'j':
                        c = '|';
                        break;
                    case 'l':
                    case 'k':
                        c = ' ';
                        break;
                    default:
                        c = '?';
                    }
                }
                if (bInsert) // insert one space
                    vt100_Escape((unsigned char *)"[1@", 3);
                if (cursor_x - line[cursor_y] >= size_x)
                {
                    int char_cnt = 0;
                    for (int i = line[cursor_y]; i < cursor_x; i++)
                        if ((buff[i] & 0xc0) != 0x80)
                            char_cnt++;
                    if (char_cnt == size_x)
                    {
                        if (bWraparound)
                            next_line();
                        else
                            cursor_x--;
                    }
                }
                attr[cursor_x] = c_attr;
                buff[cursor_x++] = c;
                if (line[cursor_y + 1] < cursor_x)
                    line[cursor_y + 1] = cursor_x;
            }
        }
    }

    const unsigned char *EmulatedTerminal::vt100_Escape(const unsigned char *sz, int cnt)
    {
        const unsigned char *zz = sz + cnt;
        bEscape = true;
        while (sz < zz && bEscape)
        {
            if (*sz > 31)
                ESC_code[ESC_idx++] = *sz++;
            else
            {
                switch (*sz++)
                {
                case 0x08:                                 // BS
                    if ((buff[cursor_x--] & 0xc0) == 0x80) // utf8 continuation byte
                        while ((buff[cursor_x] & 0xc0) == 0x80)
                            cursor_x--;
                    break;
                case 0x0b:
                { // VT
                    int x = cursor_x - line[cursor_y];
                    cursor_x = line[++cursor_y] + x;
                    break;
                }
                case 0x0d: // CR
                    cursor_x = line[cursor_y];
                    break;
                }
            }
            switch (ESC_code[0])
            {
            case '[':
                if (isalpha(ESC_code[ESC_idx - 1]) || ESC_code[ESC_idx - 1] == '@' || ESC_code[ESC_idx - 1] == '`')
                {
                    bEscape = false;
                    int m0 = 0; // used by [PsJ and [PsK
                    int n0 = 1; // used by most, e.g. [PsA [PsB
                    int n1 = 1; // n1;n0 used by [Ps;PtH [Ps;Ptr
                    if (isdigit(ESC_code[1]))
                    {
                        m0 = n0 = atoi(ESC_code + 1);
                        if (n0 == 0)
                            n0 = 1;
                    }
                    char *p = strchr(ESC_code, ';');
                    if (p != NULL)
                    {
                        n1 = n0;
                        n0 = atoi(p + 1);
                        if (n0 == 0)
                            n0 = 1; // ESC[0;0f == ESC[1;1f
                    }
                    int x;
                    switch (ESC_code[ESC_idx - 1])
                    {
                    case 'A': // cursor up n0 times
                        x = cursor_x - line[cursor_y];
                        cursor_y -= n0;
                        check_cursor_y();
                        cursor_x = line[cursor_y] + x;
                        break;
                    case 'd': // line position absolute
                        x = cursor_x - line[cursor_y];
                        if (n0 > size_y)
                            n0 = size_y;
                        cursor_y = screen_y + n0 - 1;
                        cursor_x = line[cursor_y] + x;
                        break;
                    case 'e': // line position relative
                    case 'B': // cursor down n0 times
                        x = cursor_x - line[cursor_y];
                        cursor_y += n0;
                        check_cursor_y();
                        cursor_x = line[cursor_y] + x;
                        break;
                    case '`': // character position absolute
                    case 'G': // cursor to n0th position from left
                        cursor_x = line[cursor_y];
                        // fall through
                    case 'a': // character position relative
                    case 'C': // cursor forward n0 times
                        while (n0-- > 0 && cursor_x < line[cursor_y] + size_x - 1)
                        {
                            if ((buff[++cursor_x] & 0xc0) == 0x80)
                                while ((buff[++cursor_x] & 0xc0) == 0x80)
                                    ;
                        }
                        break;
                    case 'D': // cursor backward n0 times
                        while (n0-- > 0 && cursor_x > line[cursor_y])
                        {
                            if ((buff[--cursor_x] & 0xc0) == 0x80)
                                while ((buff[--cursor_x] & 0xc0) == 0x80)
                                    ;
                        }
                        break;
                    case 'E': // cursor to begining of next line n0 times
                        cursor_y += n0;
                        check_cursor_y();
                        cursor_x = line[cursor_y];
                        break;
                    case 'F': // cursor to begining of previous line n0 times
                        cursor_y -= n0;
                        check_cursor_y();
                        cursor_x = line[cursor_y];
                        break;
                    case 'f': // horizontal/vertical position forced, apt install
                        for (int i = cursor_y + 1; i < screen_y + n1; i++)
                            if (i < LINE_SIZE && line[i] < cursor_x)
                                line[i] = cursor_x;
                        // fall through
                    case 'H': // cursor to line n1, postion n0
                        if (!bAltScreen && n1 > size_y)
                        {
                            cursor_y = (screen_y++) + size_y;
                        }
                        else
                        {
                            cursor_y = screen_y + n1 - 1;
                            if (bOriginMode)
                                cursor_y += roll_top;
                            check_cursor_y();
                        }
                        cursor_x = line[cursor_y];
                        while (--n0 > 0)
                        {
                            cursor_x++;
                            while ((buff[cursor_x] & 0xc0) == 0x80)
                                cursor_x++;
                        }
                        break;
                    case 'J': //[0J kill till end, 1J begining, 2J entire screen
                        if (isdigit(ESC_code[1]) || bAltScreen)
                        {
                            //screen_clear(m0);
                        }
                        else
                        { // clear in none alter screen, used in apt install
                            line[cursor_y + 1] = cursor_x;
                            for (int i = cursor_y + 2; i <= screen_y + size_y + 1; i++)
                                if (i < LINE_SIZE)
                                    line[i] = 0;
                        }
                        break;
                    case 'K':
                    { //[K erase till line end, 1K begining, 2K entire line
                        int a = line[cursor_y];
                        int z = line[cursor_y + 1];
                        if (m0 == 0)
                            a = cursor_x;
                        if (m0 == 1)
                            z = cursor_x + 1;
                        if (z > a)
                            buff_clear(a, z - a);
                    }
                    break;
                    case 'L': // insert n0 lines
                        if (n0 > screen_y + roll_bot - cursor_y)
                            n0 = screen_y + roll_bot - cursor_y + 1;
                        else
                            for (int i = screen_y + roll_bot; i >= cursor_y + n0; i--)
                            {
                                memcpy(buff + line[i], buff + line[i - n0], size_x);
                                memcpy(attr + line[i], attr + line[i - n0], size_x);
                            }
                        cursor_x = line[cursor_y];
                        buff_clear(cursor_x, size_x * n0);
                        break;
                    case 'M': // delete n0 lines
                        if (n0 > screen_y + roll_bot - cursor_y)
                            n0 = screen_y + roll_bot - cursor_y + 1;
                        else
                            for (int i = cursor_y; i <= screen_y + roll_bot - n0; i++)
                            {
                                memcpy(buff + line[i], buff + line[i + n0], size_x);
                                memcpy(attr + line[i], attr + line[i + n0], size_x);
                            }
                        cursor_x = line[cursor_y];
                        buff_clear(line[screen_y + roll_bot - n0 + 1], size_x * n0);
                        break;
                    case 'P': // delete n0 characters
                        for (int i = cursor_x + n0; i < line[cursor_y + 1]; i++)
                        {
                            buff[i - n0] = buff[i];
                            attr[i - n0] = attr[i];
                        }
                        buff_clear(line[cursor_y + 1] - n0, n0);
                        if (!bAltScreen)
                        {
                            line[cursor_y + 1] -= n0;
                            if (line[cursor_y + 1] < line[cursor_y])
                                line[cursor_y + 1] = line[cursor_y];
                        }
                        break;
                    case '@': // insert n0 spaces
                        for (int i = line[cursor_y + 1] - n0 - 1; i >= cursor_x; i--)
                        {
                            buff[i + n0] = buff[i];
                            attr[i + n0] = attr[i];
                        }
                        if (!bAltScreen)
                        {
                            line[cursor_y + 1] += n0;
                            if (line[cursor_y + 1] > line[cursor_y] + size_x)
                                line[cursor_y + 1] = line[cursor_y] + size_x;
                        }     // fall through
                    case 'X': // erase n0 characters
                        buff_clear(cursor_x, n0);
                        break;
                    case 'I': // cursor forward n0 tab stops
                        break;
                    case 'Z': // cursor backward n0 tab stops
                        break;
                    case 'S': // scroll up n0 lines
                        for (int i = roll_top; i <= roll_bot - n0; i++)
                        {
                            memcpy(buff + line[screen_y + i],
                                   buff + line[screen_y + i + n0], size_x);
                            memcpy(attr + line[screen_y + i],
                                   attr + line[screen_y + i + n0], size_x);
                        }
                        buff_clear(line[screen_y + roll_bot - n0 + 1], n0 * size_x);
                        break;
                    case 'T': // scroll down n0 lines
                        for (int i = roll_bot; i >= roll_top + n0; i--)
                        {
                            memcpy(buff + line[screen_y + i],
                                   buff + line[screen_y + i - n0], size_x);
                            memcpy(attr + line[screen_y + i],
                                   attr + line[screen_y + i - n0], size_x);
                        }
                        buff_clear(line[screen_y + roll_top], n0 * size_x);
                        break;
                    case 'c':               // send device attributes
                        //send("\033[?1;2c"); // vt100 with options
                        break;
                    case 'g': // set tabstops
                        if (m0 == 0)
                        { // clear current tab
                            tabstops[cursor_x - line[cursor_y]] = 0;
                        }
                        if (m0 == 3)
                        { // clear all tab stops
                            memset(tabstops, 0, 256);
                        }
                        break;
                    case 'h':
                        if (ESC_code[1] == '4')
                            bInsert = true;
                        if (ESC_code[1] == '?')
                        {
                            switch (atoi(ESC_code + 2))
                            {
                            case 1:
                                bAppCursor = true;
                                break;
                            case 3:
                                //termsize(132, 25);
                                break;
                            case 6:
                                bOriginMode = true;
                                break;
                            case 7:
                                bWraparound = true;
                                break;
                            case 25:
                                bCursor = true;
                                break;
                            case 2004:
                                bBracket = true;
                                break;
                            //case 1049:
                                // bAltScreen = true; //?1049h alternate screen
                                //screen_clear(2);
                            }
                        }
                        break;
                    case 'l':
                        if (ESC_code[1] == '4')
                            bInsert = false;
                        if (ESC_code[1] == '?')
                        {
                            switch (atoi(ESC_code + 2))
                            {
                            case 1:
                                bAppCursor = false;
                                break;
                            case 3:
                                //termsize(80, 25);
                                break;
                            case 6:
                                bOriginMode = false;
                                break;
                            case 7:
                                bWraparound = false;
                                break;
                            case 25:
                                bCursor = false;
                                break;
                            case 2004:
                                bBracket = false;
                                break;
                            case 1049:
                                bAltScreen = false; //?1049l alternate screen
                                cursor_y = screen_y;
                                cursor_x = line[cursor_y];
                                for (int i = 1; i <= size_y + 1; i++)
                                    line[cursor_y + i] = 0;
                                screen_y = cursor_y - size_y + 1;
                                if (screen_y < 0)
                                    screen_y = 0;
                            }
                        }
                        break;
                    case 'm':
                    { // text style, color attributes
                        char *p = ESC_code;
                        while (p != NULL)
                        {
                            m0 = atoi(++p);
                            switch (m0 / 10)
                            {
                            case 0:
                                if (m0 == 0)
                                    c_attr = 7; // normal
                                if (m0 == 1)
                                    c_attr |= 0x08; // bright
                                if (m0 == 7)
                                    c_attr = 0x70; // negative
                                break;
                            case 2:
                                c_attr = 7; // normal
                                break;
                            case 3:
                                if (m0 == 39)
                                    m0 = 7; // default foreground
                                c_attr = (c_attr & 0xf8) + m0 % 10;
                                break;
                            case 4:
                                if (m0 == 49)
                                    m0 = 0; // default background
                                c_attr = (c_attr & 0x0f) + ((m0 % 10) << 4);
                                break;
                            case 9:
                                c_attr = (c_attr & 0xf0) + m0 % 10 + 8;
                                break;
                            case 10:
                                c_attr = (c_attr & 0x0f) + ((m0 % 10 + 8) << 4);
                                break;
                            }
                            p = strchr(p, ';');
                        }
                    }
                    break;
                    case 'r': // set margins and move cursor to home
                        if (n1 == 1 && n0 == 1)
                            n0 = size_y; // ESC[r
                        roll_top = n1 - 1;
                        roll_bot = n0 - 1;
                        cursor_y = screen_y;
                        if (bOriginMode)
                            cursor_y += roll_top;
                        cursor_x = line[cursor_y];
                        break;
                    case 's': // save cursor
                        save_x = cursor_x - line[cursor_y];
                        save_y = cursor_y - screen_y;
                        break;
                    case 'u': // restore cursor
                        cursor_y = save_y + screen_y;
                        cursor_x = line[cursor_y] + save_x;
                        break;
                    }
                }
                break;
            case '7': // save cursor
                save_x = cursor_x - line[cursor_y];
                save_y = cursor_y - screen_y;
                save_attr = c_attr;
                bEscape = false;
                break;
            case '8': // restore cursor
                cursor_y = save_y + screen_y;
                cursor_x = line[cursor_y] + save_x;
                c_attr = save_attr;
                bEscape = false;
                break;
            case 'F': // cursor to lower left corner
                cursor_y = screen_y + size_y - 1;
                cursor_x = line[cursor_y];
                bEscape = false;
                break;
            case 'E': // move to next line
                cursor_x = line[++cursor_y];
                bEscape = false;
                break;
            case 'D': // move/scroll up one line
                if (cursor_y < screen_y + roll_bot)
                { // move
                    int x = cursor_x - line[cursor_y];
                    cursor_x = line[++cursor_y] + x;
                }
                else
                { // scroll
                    int len = line[screen_y + roll_bot + 1] - line[screen_y + roll_top + 1];
                    int x = cursor_x - line[cursor_y];
                    memcpy(buff + line[screen_y + roll_top],
                           buff + line[screen_y + roll_top + 1], len);
                    memcpy(attr + line[screen_y + roll_top],
                           attr + line[screen_y + roll_top + 1], len);
                    len = line[screen_y + roll_top + 1] - line[screen_y + roll_top];
                    for (int i = roll_top + 1; i <= roll_bot; i++)
                        line[screen_y + i] = line[screen_y + i + 1] - len;
                    buff_clear(line[screen_y + roll_bot],
                               line[screen_y + roll_bot + 1] - line[screen_y + roll_bot]);
                    cursor_x = line[cursor_y] + x;
                }
                bEscape = false;
                break;
            case 'M': // move/scroll down one line
                if (cursor_y > screen_y + roll_top)
                { // move
                    int x = cursor_x - line[cursor_y];
                    cursor_x = line[--cursor_y] + x;
                }
                else
                { // scroll
                    for (int i = roll_bot; i > roll_top; i--)
                    {
                        memcpy(buff + line[screen_y + i], buff + line[screen_y + i - 1], size_x);
                        memcpy(attr + line[screen_y + i], attr + line[screen_y + i - 1], size_x);
                    }
                    buff_clear(line[screen_y + roll_top], size_x);
                }
                bEscape = false;
                break;
            case 'H': // set tabstop
                tabstops[cursor_x - line[cursor_y]] = 1;
                bEscape = false;
                break;
            case ']': // set window title
                if (ESC_code[ESC_idx - 1] == ';')
                {
                    if (ESC_code[1] == '0')
                    {
                        bTitle = true;
                    }
                    bEscape = false;
                }
                break;
            case ')':
            case '(': // character sets, 0 for line drawing
                if (ESC_idx == 2)
                {
                    bGraphic = (ESC_code[1] == '0');
                    bEscape = false;
                }
                break;
            case '#':
                if (ESC_idx == 2)
                {
                    if (ESC_code[1] == '8')
                        memset(buff + line[screen_y], 'E', size_x * size_y);
                    bEscape = false;
                }
                break;
            default:
                bEscape = false;
            }
            if (ESC_idx == 31)
                bEscape = false;
            if (!bEscape)
            {
                ESC_idx = 0;
                memset(ESC_code, 0, 32);
            }
        }
        return sz;
    }

    void EmulatedTerminal::buff_clear(int offset, int len)
    {
        memset(buff + offset, ' ', len);
        memset(attr + offset, 7, len);
    }

    void EmulatedTerminal::next_line()
    {
        line[++cursor_y] = cursor_x;
        if (screen_y == cursor_y - size_y)
            screen_y++;
        if (line[cursor_y + 1] < cursor_x)
            line[cursor_y + 1] = cursor_x;
        if (cursor_x > BUFFER_SIZE - 1024 || cursor_y > LINE_SIZE - 3)
        {
            int middle = line[HALF_LINES];
            for (int i = HALF_LINES; i < cursor_y + 2; i++)
                line[i] -= middle;
            memmove(line, line + HALF_LINES, HALF_LINES * sizeof(int));
            memset(line + HALF_LINES, 0, HALF_LINES * sizeof(int));
            screen_y -= HALF_LINES;
            if (screen_y < 0)
                screen_y = 0;
            cursor_y -= HALF_LINES;
            cursor_x -= middle;
            recv0 -= middle;
            if (recv0 < 0)
                recv0 = 0;
            memmove(attr, attr + middle, line[cursor_y + 1]);
            memset(attr + line[cursor_y + 1], 0, LINE_COUNT * 64 - line[cursor_y + 1]);
            memmove(buff, buff + middle, line[cursor_y + 1]);
            memset(buff + line[cursor_y + 1], 0, LINE_COUNT * 64 - line[cursor_y + 1]);
        }
    }

    void EmulatedTerminal::check_cursor_y()
    {
        if (cursor_y < screen_y)
            cursor_y = screen_y;
        if (cursor_y > screen_y + size_y - 1)
            cursor_y = screen_y + size_y - 1;
        if (bOriginMode)
        {
            if (cursor_y < screen_y + roll_top)
                cursor_y = screen_y + roll_top;
            if (cursor_y > screen_y + roll_bot)
                cursor_y = screen_y + roll_bot;
        }
    }

    const unsigned int VT_attr[] = {
        0x00000000, 0xc0000000, 0x00c00000, 0xc0c00000, // 0,1,2,3
        0x2060c000, 0xc000c000, 0x00c0c000, 0xc0c0c000, // 4,5,6,7
        FL_BLACK, FL_RED, FL_GREEN, FL_YELLOW,
        FL_BLUE, FL_MAGENTA, FL_CYAN, FL_WHITE};

    void EmulatedTerminal::draw(Fl_Widget *target, int font_face, int font_size, int font_height)
    {
        std::unique_lock<std::mutex> lk(mtx_);
        
        fl_push_clip(target->x(), target->y(), target->w(), target->h());

        fl_color(target->color());
        fl_rectf(target->x(), target->y(), target->w(), target->h());
        fl_font(font_face, font_size);

        int sel_l = 0, sel_r = 0;
        int ly = screen_y;
        int dx, dy = target->y();

        int last_line = 0;
        for (int i = 0; i < size_y; i++) {
            if (line[ly + i] < line[ly + i + 1]) {
                ++last_line;
            };
        }
        
        int cap = (target->h() - font_height / 2) / font_height;
        int scroll_y = last_line - cap;
        if (scroll_y < 0) {
            scroll_y = 0;
        }

        for (int i = scroll_y; i < size_y; i++)
        {
            dx = target->x() + 1;
            dy += font_height;
            int j = line[ly + i];
            while (j < line[ly + i + 1])
            {
                int n = j;
                while (attr[n] == attr[j])
                {
                    if (++n == line[ly + i + 1])
                        break;
                    if (n == sel_r || n == sel_l)
                        break;
                }
                unsigned int font_color = VT_attr[(int)attr[j] & 0x0f];
                unsigned int bg_color = VT_attr[(int)((attr[j] >> 4) & 0x0f)];
                int wi = fl_width(buff + j, n - j);

                if (bg_color != target->color())
                {
                    fl_color(bg_color);
                    fl_rectf(dx, dy - font_height + 4, wi, font_height);
                }
                fl_color(font_color);
                int m = (buff[n - 1] == 0x0a) ? n - 1 : n; // don't draw LF,
                // which will result in little squares on some platforms
                fl_draw(buff + j, m - j, dx, dy);
                dx += wi;
                j = n;
            }
        }

        fl_pop_clip();
    }

    void EmulatedTerminal::copy_to_clipboard() {
        std::unique_lock<std::mutex> lk(mtx_);
        std::string contents;
        int sz = 0;
        int j = 0;
        for (int i = 0; i < size_y; i++)
        {
            j = line[i];
            sz = line[i + 1] - j;
            if (sz > 0) {
                contents += std::string(&buff[j], sz);
            }
        }
        if (!contents.empty()) {
#ifdef _WIN32
        HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, contents.size());
        if (hMem != NULL) {
            memcpy(GlobalLock(hMem), &contents[0], contents.size());
            GlobalUnlock(hMem);
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
        }
#endif
        }
    }

} // namespace dexpert
