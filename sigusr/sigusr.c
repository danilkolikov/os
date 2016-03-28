#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handler(int signal, siginfo_t* info, void* sth) {
    char* signame;
    if (signal == SIGUSR1) {
        signame = "SIGUSR1";
    } else {
        signame = "SIGUSR2";
    }
    printf("%s from %d\n", signame, info->si_pid);
}

int main() {
    struct sigaction act;
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &act, NULL) != 0 || 
            sigaction(SIGUSR2, &act, NULL) != 0) {
        printf("Can't set signal handlers\n");
        return 1;
    }

    if (sleep(10) == 0) {
        printf("No signals were caught\n");
    }
    return 0;
}
