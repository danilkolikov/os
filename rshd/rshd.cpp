#include <cstdio>
#include "rshd_server.h"

const char *PID_PATH = "/tmp/rshd.pid";

void start_server(uint16_t port) {
    try {
        epoll_wrap epoll(100);
        signal_fd signal({SIGCHLD, SIGINT, SIGTTIN}, {signal_fd::NONBLOCK});
        epoll_registration signal_registration(epoll, std::move(signal), fd_state::IN);
        signal_registration.update([&epoll, &signal_registration](fd_state state) {
            if (state.is(fd_state::IN)) {
                struct signalfd_siginfo sinf;
                long size = signal_registration.get_fd().read(&sinf, sizeof(struct signalfd_siginfo));
                if (size != sizeof(struct signalfd_siginfo)) {
                    return;
                }
                if (sinf.ssi_signo == SIGINT) {
//                printf("Server stopped\n");
                    epoll.stop_wait();
                }
                if (sinf.ssi_signo == SIGCHLD || sinf.ssi_signo == SIGTTIN) {
                    // Nothing to do here
                }
            }
        });
        rshd_server rshd(epoll, port);

        epoll.start_wait();
    } catch (annotated_exception const &e) {
        printf("%s", e.what());
    }
}

void daemonise(uint16_t port) {
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        pid_t daemon_parent = fork();
        if (daemon_parent == 0) {
            int file = creat(PID_PATH, O_WRONLY);
            if (file == -1) {
                printf("Failed to create *.pid file\n");
                exit(0);
            }
            pid = getpid();
            std::string pid_string = std::to_string(pid);
            write(file, pid_string.c_str(), pid_string.size());
            close(file);

            chdir("/");
            start_server(port);
            remove(PID_PATH);
        } else {
            exit(0);
        }
    } else {
        exit(0);
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("No arguments\n");
        return 1;
    }
    int port = std::stoi(argv[1]);
    daemonise(port);
    return 0;
}