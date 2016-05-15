#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void set_sigaction(struct sigaction *act) {
    for (int i = 1; i < 32; i++) {
        if (i == SIGKILL || i == SIGSTOP) {
            continue;
        }
        if (sigaction(i, act, NULL) != 0) {
            printf("Can't set signal handler %d\n", i);
        }
    }
}

void handler(int signal, siginfo_t* info, void* sth) {
    struct sigaction act;
    memset(&act, 0, sizeof act);
    set_sigaction(&act);

    printf("%d from %d\n", signal, info->si_pid);
}

int main() {
    struct sigaction act;
    act.sa_sigaction = handler;
   
    sigset_t mask;
    sigemptyset(&mask);
    for (int i = 1; i < 32; i++) {
        if (i == SIGKILL || i == SIGSTOP) {
            continue;
        }
        sigaddset(&mask, i);
    }
    act.sa_mask = mask;
    act.sa_flags = SA_SIGINFO;
    
    set_sigaction(&act);
    if (sleep(10) == 0) {
        printf("No signals were caught\n");
    }
    return 0;
}
