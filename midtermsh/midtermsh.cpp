#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <vector>
#include <errno.h>
#include <stdlib.h>

void print(char* s) {
    int written = 0;
    int part;
    int size = strlen(s);
    while ((part = write(0, s + written, size - written)) > 0) {
        written += part;
    }
}

void print_error() {
    print(strerror(errno));
    print("\n");
    exit(errno);
}

using namespace std;

bool new_line = false;
string read_token() {
    if (new_line) {
        new_line = false;
        return "\n";
    }

    char buffer[64];
    memset(&buffer, 0, sizeof buffer);

    int c;
    int pos = 0;
    while (c = getchar()) {
        if (c == '\n') {
            new_line = true;
        }
        if (c == ' ' && pos == 0) {
            continue;
        }
        if (c == -1 || c == 0 || c == ' ' || c == '\n') {
            break;
        }
        buffer[pos++] = c;
    }

    return string(buffer);
}

typedef vector<string> command_t;

command_t read_command() {
    vector<string> res;
    string token;

    while (true) {
        token = read_token();
        res.push_back(token);
        if (token == "|" || token == "\n") {
            break;
        }
    }
    return res;
}

vector<command_t> read_pipe() {
    vector<command_t> pipe;
    
    while (true) {
        command_t cmd = read_command();
        if (cmd.back() == "\n") {
            cmd.pop_back();
            pipe.push_back(cmd);
            break;
        }
        if (cmd.back() == "|") {
            cmd.pop_back();
        }
        pipe.push_back(cmd);
    }
    return pipe;
}

void exec(command_t const& cmd) {
    char * args[cmd.size() + 1];
    for (size_t i = 0; i < cmd.size(); i++) {
        args[i] = const_cast<char*>(cmd[i].c_str());
    }
    args[cmd.size()] = NULL;
    
    if (execvp(cmd[0].c_str(), args) != 0) {
        print_error();
    }
}

vector<int> cur_command;

void exec(vector<command_t> const& cmd_pipe, int pos, int prev_stdin) {
    if (pos + 1 == cmd_pipe.size()) {
        pid_t pid = fork();
        
        if (pid != 0) {
            cur_command.push_back(pid);
        }

        if (pid == 0) {
            dup2(prev_stdin, STDIN_FILENO);
            if (prev_stdin != 0) {
                close(prev_stdin);
            }
            exec(cmd_pipe[pos]);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
        return;
    }

    int pipes[2];
    if (pipe(pipes)) {
        print_error();
    }

    pid_t pid = fork();
    if (pid != 0) {
        cur_command.push_back(pid);
    }
    if (pid == 0) {
        dup2(pipes[1], STDOUT_FILENO);
        dup2(prev_stdin, STDIN_FILENO);

        close(pipes[0]);
        close(pipes[1]);
        if (prev_stdin != 0) {
            close(prev_stdin);
        }
        exec(cmd_pipe[pos]);
    } else {
        close(pipes[1]);
        exec(cmd_pipe, pos + 1, pipes[0]);
        close(pipes[0]);
        if (prev_stdin != 0) {
            close(prev_stdin);
        }
        int status;
        waitpid(pid, &status, 0);
    }
    return;
}

void handle_sigint(int sig) {
    for (size_t i = 0; i < cur_command.size(); i++) {
        kill(cur_command[i], SIGINT);
    }
}

int main() {
    struct sigaction act;
    memset(&act, 0, sizeof act);
    
    act.sa_handler = handle_sigint;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    act.sa_mask = mask;
    if (sigaction(SIGINT, &act, NULL)) {
        print_error();
    }

    while (true) {
        print("$ ");
        cur_command.clear();
        vector<command_t> pipe = read_pipe();
        exec(pipe, 0, 0);
    }
    return 0;
}
