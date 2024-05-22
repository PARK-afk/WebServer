#include "../Server/WebServer.hpp"
#include "../Parse/Config.hpp"
#include "../utils/Error.hpp"
#include <signal.h>

#include <csignal>

// 특정 신호를 무시하도록 설정하는 함수
void ignoreSignal(int signum) {
    std::signal(signum, SIG_IGN);
}

// 모든 신호를 무시하도록 설정하는 함수
void ignoreAllSignals() {
    // 무시할 신호 목록
    int signals[] = {
        SIGINT,   // Interrupt (Ctrl + C)
        SIGTERM,  // Termination
        SIGQUIT,  // Quit (Ctrl + \)
        SIGHUP,   // Hangup detected on controlling terminal or death of controlling process
        SIGPIPE,  // Write on a pipe with no one to read it
        SIGALRM,  // Alarm clock
        SIGUSR1,  // User-defined signal 1
        SIGUSR2,  // User-defined signal 2
        SIGCHLD,  // Child process stopped or terminated
        SIGCONT,  // Continue if stopped
        SIGSTOP,  // Stop process
        SIGTSTP,  // Stop typed at terminal
        SIGTTIN,  // Terminal input for background process
        SIGTTOU,  // Terminal output for background process
        SIGABRT,  // Abnormal termination
        SIGSEGV,  // Invalid memory reference
        SIGFPE,   // Floating-point exception
        SIGILL,   // Illegal instruction
        SIGBUS,   // Bus error (bad memory access)
        SIGTRAP,  // Trace/breakpoint trap
        SIGURG,   // Urgent condition on socket
        SIGXCPU,  // CPU time limit exceeded
        SIGXFSZ,  // File size limit exceeded
        SIGVTALRM,// Virtual alarm clock
        SIGPROF,  // Profiling timer expired
        SIGWINCH, // Window resize signal
        SIGSYS    // Bad system call
    };

    // 신호를 무시하도록 설정
    for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
        ignoreSignal(signals[i]);
    }
}


int main(int ac, char **av)
{
	ignoreAllSignals();
	Error::initializeError();
	Config Conf(ac, av);
	Server Server;
	Server.makeServerSocket(Conf);
	Server.queueInit(Conf);
	Server.run(Conf);
}
