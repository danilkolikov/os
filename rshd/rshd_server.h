//
// Created by danil on 14.05.16.
//

#ifndef RSHD_DAEMON_H
#define RSHD_DAEMON_H
#include "wraps.h"

#include <string>
#include <vector>
#include <utility>
#include "stdlib.h"
#include "raw_message.h"
#include "unistd.h"
#include <termios.h>
#include "sys/wait.h"

struct rshd_server {
    rshd_server(epoll_wrap& epoll, uint16_t port);
    rshd_server(rshd_server const& other) = delete;
    rshd_server(rshd_server &&other) = delete;
    ~rshd_server() = default;

private:
    using clients_t = std::map<int, epoll_registration>;
    using iterator_t = clients_t::iterator;
    using pipe_t = std::pair<iterator_t , iterator_t>;
    using shared_message = std::shared_ptr<raw_message>;

    clients_t clients;
    std::map<int, int> terminals;
    epoll_wrap &epoll;
    epoll_registration listener;

    iterator_t make_terminal();

    epoll_wrap::handler_t make_server_handler();
    epoll_wrap::handler_t make_tty_handler(iterator_t tty, iterator_t client, shared_message in, shared_message out);
    epoll_wrap::handler_t make_client_handler(iterator_t client, iterator_t tty, shared_message in, shared_message out);
    void close_client(iterator_t const &client, iterator_t const &tty);
};
#endif //RSHD_DAEMON_H
