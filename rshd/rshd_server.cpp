#include "rshd_server.h"

rshd_server::rshd_server(epoll_wrap &epoll, uint16_t port) :
        epoll(epoll) {
    socket_wrap socket(socket_wrap::NONBLOCK);
    socket.bind(port);
    socket.listen(epoll.get_queue_size());
    listener = epoll_registration(epoll, std::move(socket), fd_state::IN, make_server_handler());
}

epoll_wrap::handler_t rshd_server::make_server_handler() {
    return [this](fd_state state) {
        if (state.is(fd_state::IN)) {
            socket_wrap &server = *static_cast<socket_wrap *>(&listener.get_fd());
            socket_wrap client = server.accept(socket_wrap::NONBLOCK);
            epoll_registration registration(epoll, std::move(client), {fd_state::IN, fd_state::RDHUP});
            auto it = clients.insert(std::make_pair(registration.get_fd().get(), std::move(registration))).first;
            auto tty = make_terminal();

//            printf("New client %d with terminal %d accepted\n", it->second.get_fd().get(),
//                   tty->second.get_fd().get());

            shared_message in = std::make_shared<raw_message>();
            shared_message out = std::make_shared<raw_message>();

            it->second.update(fd_state::RDHUP, make_client_handler(it, tty, in, out));
            tty->second.update({fd_state::RDHUP, fd_state::IN}, make_tty_handler(tty, it, out, in));
        }
    };
}

epoll_wrap::handler_t rshd_server::make_client_handler(iterator_t client, iterator_t tty, shared_message in,
                                                       shared_message out) {
    return [this, client, tty, in, out](fd_state state) {
        file_descriptor const &fd = client->second.get_fd();

        if (state.is(fd_state::RDHUP) || state.is(fd_state::HUP) || state.is(fd_state::ERROR)) {
//            printf("Client %d disconnected\n", fd.get());
            close_client(client, tty);
            return;
        }
        if (state.is(fd_state::IN) && in->can_read()) {
            try {
                in->read_from(fd);
            } catch (annotated_exception const &e) {
                log(e);
                close_client(client, tty);
                return;
            }
            if (!in->can_read()) {
                client->second.update(client->second.get_state() ^ fd_state::IN);
            }
            if (in->can_write()) {
                tty->second.update(tty->second.get_state() | fd_state::OUT);
            }
            if (out->can_write()) {
                client->second.update(client->second.get_state() | fd_state::OUT);
            }
            return;
        }
        if (state.is(fd_state::OUT) && out->can_write()) {
            try {
                out->write_to(fd);
            } catch (annotated_exception const &e) {
                log(e);
                close_client(client, tty);
                return;
            }
            if (!out->can_write()) {
                client->second.update(client->second.get_state() ^ fd_state::OUT);
            }
            if (out->can_read()) {
                tty->second.update(tty->second.get_state() | fd_state::IN);
            }
            if (in->can_read()) {
                client->second.update(client->second.get_state() | fd_state::IN);
            }
            return;
        }
    };
}

epoll_wrap::handler_t rshd_server::make_tty_handler(iterator_t tty, iterator_t client, shared_message in,
                                                    shared_message out) {
    return [this, tty, client, in, out](fd_state state) {
        file_descriptor const &fd = tty->second.get_fd();
        if (state.is(fd_state::RDHUP) || state.is(fd_state::HUP) || state.is(fd_state::ERROR)) {
//            printf("Session with %d closed\n", client->second.get_fd().get());
            close_client(client, tty);
            return;
        }

        if (state.is(fd_state::IN) && in->can_read()) {
            try {
                in->read_from(fd);
            } catch (annotated_exception const &e) {
                log(e);
                close_client(client, tty);
                return;
            }
            if (!in->can_read()) {
                tty->second.update(tty->second.get_state() ^ fd_state::IN);
            }
            if (in->can_write()) {
                client->second.update(client->second.get_state() | fd_state::OUT);
            }
            if (out->can_write()) {
                tty->second.update(tty->second.get_state() | fd_state::OUT);
            }
            return;
        }
        if (state.is(fd_state::OUT) && out->can_write()) {
            try {
                out->write_to(fd);
            } catch (annotated_exception const &e) {
                log(e);
                close_client(client, tty);
                return;
            }
            if (!out->can_write()) {
                tty->second.update(tty->second.get_state() ^ fd_state::OUT);
            }
            if (out->can_read()) {
                client->second.update(client->second.get_state() | fd_state::IN);
            }
            if (in->can_read()) {
                tty->second.update(tty->second.get_state() | fd_state::IN);
            }
            return;
        }

    };
}

rshd_server::iterator_t rshd_server::make_terminal() {
    int master;
    if ((master = posix_openpt(O_RDWR)) == -1) {
        throw annotated_exception("posix_openpt", errno);
    }
    if (grantpt(master) == -1) {
        throw annotated_exception("grantpt", errno);
    }
    if (unlockpt(master) == -1) {
        throw annotated_exception("inlockpt", errno);
    }
    int slave = open(ptsname(master), O_RDWR);
    pid_t pid = fork();
    if (pid == 0) {
        struct termios settings;
        tcgetattr(slave, &settings);
        cfmakeraw(&settings);
        tcsetattr(slave, TCSANOW, &settings);

        close(master);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        close(slave);

        setsid();
        ioctl(0, TIOCSCTTY, 1);

        execlp("/bin/sh", "sh", NULL);
    } else {
        close(slave);
        epoll_registration registration(epoll, file_descriptor(master), fd_state::RDHUP);
        terminals.insert({registration.get_fd().get(), pid});

        return clients.insert(std::make_pair(master, std::move(registration))).first;
    }
}

void rshd_server::close_client(iterator_t const &client, iterator_t const &tty) {
    clients.erase(client);

    int fd = tty->first;
    auto terminal = terminals.find(fd);
    int status;
    kill(terminal->second, SIGKILL);
    waitpid(terminal->second, &status, 0);
    terminals.erase(terminal);

    clients.erase(tty);
}