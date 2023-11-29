#include <stdio.h>
#include <iostream>
#include <mutex>
#include <list>
#include <chrono>
#include <memory>
#include <thread>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

#define OUT_BUFF_SIZE 512
#define LINE_SIZE 1024
#define LINE_COUNT 512
#define HALF_LINES 256
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

#define STREAM(X) ((X) ? stdout: stderr)
#define FILENOS(X) (FILENO(STREAM(X)))


#include "terminal/terminal.h"

namespace dfe
{
    class TerminalThread {
    public:
        TerminalThread();
        ~TerminalThread();
    private:
        void process(bool standard_output);
    private:
        std::unique_ptr<std::thread> thread1_;
        std::unique_ptr<std::thread> thread2_;
        bool stopped_ = false;
    };
    
    namespace {
        int fdstout[2] = { 0, };
        int fdstderr[2] = { 0, };
        int originalstdout = 0;
        int originalstderr = 0;
        std::mutex terminal_mutex;
        std::list<Terminal *> terminals;
        TerminalThread terminal_thread;
    }

    bool redirect_stdout_stderr() {
        auto init = [](bool stream_stdout, int *pipefd, int *original_fd) -> bool {
            printf("Redirecting %s....\n", stream_stdout ? "stdout" : "stderr");
            if (MAKE_PIPE(pipefd) == -1)
            {
                puts("Failed to create pipe");
                return false;
            }
            *original_fd = DUP(FILENOS(stream_stdout));
            if (DUP2(pipefd[1], FILENOS(stream_stdout)) == -1) {
                puts("Failed to dup pipe");
                return false;
            }
            setvbuf(STREAM(stream_stdout), NULL, _IONBF, 0);    
            return true;
        };
        return init(true, fdstout, &originalstdout) && init(false, fdstderr, &originalstderr);
    };

    TerminalThread::TerminalThread() {
        if (redirect_stdout_stderr()) {
            thread1_.reset(new std::thread([
                this    
            ] () {
                process(true);
            }));
            thread2_.reset(new std::thread([
                this    
            ] () {
                process(false);
            }));
        }
    }

    TerminalThread::~TerminalThread() {
        stopped_ = true;
        if (thread1_) {
            fprintf(STREAM(true), "\n");
            fflush(STREAM(true));
            thread1_->join();
        }
        if (thread2_) {
            fprintf(STREAM(false), "\n");
            fflush(STREAM(false));
            thread2_->join();
        }
    }

    void TerminalThread::process(bool standard_output) {
        int *fd = standard_output ? fdstout : fdstderr;
        int count = 0;
        int readed = 0;
        char temp[LINE_SIZE] = {
            0,
        };
        int original_fd = standard_output ? originalstdout : originalstderr;
        while (!stopped_) {
            readed = READ(fd[0], temp, sizeof(temp) - 1);
            if (!stopped_ && readed > 0)
            {
                WRITE(original_fd, temp, readed);
                std::unique_lock<std::mutex> lk(terminal_mutex);
                for (auto *v : terminals) {
                    temp[readed] = '\0';
                    v->append(temp);
                }
            }
        }
        CLOSE(fd[0]);
        CLOSE(fd[1]);
    }

    Terminal::Terminal() {
        std::unique_lock<std::mutex> lk(terminal_mutex);
        terminals.push_back(this);
    }

    Terminal::~Terminal() {
        std::unique_lock<std::mutex> lk(terminal_mutex);
        terminals.remove(this);
    }

} // namespace dfe
