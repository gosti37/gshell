#ifndef PSCXX_HPP
#define PSCXX_HPP

#define _XOPEN_SOURCE 600

#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/poll.h>
#include <sys/wait.h>
#include <vector>
#include <optional>
#include <unistd.h>
#include <cerrno>
#include <spawn.h>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>

extern char **environ;

namespace pscxx

{

struct PRESULT
{
    int exitCode;
    int signal;
    std::string outStd;
    std::string errStd;
};

inline PRESULT execute(std::vector<std::string> execution_argv, std::string binary_path, std::optional<char *const*> environment = static_cast<char *const*>(environ), std::optional<bool> live_print = true)
{
    std::vector<char*> argv;

    for (const auto& arg : execution_argv)
    {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    int pipe_stdout[2];
    int pipe_stderr[2];
    int pipe_stdin[2];

    if (pipe(pipe_stdout) == -1)
    {
        return (PRESULT){EPIPE, 0, "", std::strerror(EPIPE)};
    }
    if (pipe(pipe_stderr) == -1)
    {
        return (PRESULT){EPIPE, 0, "", std::strerror(EPIPE)};
    }
    if (pipe(pipe_stdin) == -1)
    {
        return (PRESULT){EPIPE, 0, "", std::strerror(EPIPE)};
    }

    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);

    posix_spawn_file_actions_adddup2(&action, pipe_stdout[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&action, pipe_stderr[1], STDERR_FILENO);
    posix_spawn_file_actions_adddup2(&action, pipe_stdin[0], STDIN_FILENO);

    posix_spawn_file_actions_addclose(&action, pipe_stdout[0]);
    posix_spawn_file_actions_addclose(&action, pipe_stderr[0]);
    posix_spawn_file_actions_addclose(&action, pipe_stdin[0]);
    posix_spawn_file_actions_addclose(&action, pipe_stdout[1]);
    posix_spawn_file_actions_addclose(&action, pipe_stderr[1]);
    posix_spawn_file_actions_addclose(&action, pipe_stdin[1]);

    pid_t pid;
    int statusCode;

    try 
    {
        statusCode = posix_spawn(&pid, binary_path.c_str(), &action, NULL, argv.data(), *environment);
    }
    catch (std::bad_optional_access& e) 
    {
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        close(pipe_stdin[0]);
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);
        close(pipe_stdin[1]);
        return (PRESULT){1, 0, "", e.what()};
    }

    posix_spawn_file_actions_destroy(&action);
    close(pipe_stdout[1]);
    close(pipe_stderr[1]);
    close(pipe_stdin[0]);

    if (statusCode != 0)
    {
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        close(pipe_stdin[1]);
        return (PRESULT){statusCode, 0, "", std::strerror(statusCode)};
    }

    int statusWait;

    std::string out;
    std::string err;
    char buff_out[256];
    char buff_err[256];
    char buff_in[256];
    ssize_t bytes_read_out;
    ssize_t bytes_read_err;
    ssize_t bytes_read_in;

    struct pollfd fd_polls[3];

    fd_polls[0].fd = pipe_stdout[0];
    fd_polls[1].fd = pipe_stderr[0];
    fd_polls[2].fd = STDIN_FILENO;

    for (int i = 0; i < 3; i++)
    {
        fd_polls[i].events = POLLIN;
        fd_polls[i].revents = 0;
    }

    bool stdout_active = true;
    bool stderr_active = true;
    bool stdin_active = true;

    while (stdout_active || stderr_active)
    {
        poll(fd_polls, 3, -1);

        for (int i = 0; i < 3; i++)
        {
            if (fd_polls[i].revents & (POLLIN | POLLHUP | POLLERR) && fd_polls[i].fd == STDIN_FILENO && stdin_active)
            {
                bytes_read_in = read(STDIN_FILENO, buff_in, sizeof(buff_in));
                if (bytes_read_in > 0)
                {
                    write(pipe_stdin[1], buff_in, bytes_read_in);
                }
                else
                {
                    stdin_active = false;
                    close(pipe_stdin[1]);
                    fd_polls[i].fd = -1;
                }
            }
            if (fd_polls[i].revents & (POLLIN | POLLHUP | POLLERR) && fd_polls[i].fd == pipe_stdout[0] && stdout_active)
            {
                if ((bytes_read_out = read(pipe_stdout[0], buff_out, sizeof(buff_out) - 1)) > 0)
                {
                    buff_out[bytes_read_out] = '\0';
                    if (*live_print) { write(STDOUT_FILENO, buff_out, bytes_read_out); }
                    out.append(buff_out);
                }
                else
                {
                    fd_polls[i].fd = -1;
                    stdout_active = false; 
                }
            }

            if (fd_polls[i].revents & (POLLIN | POLLHUP | POLLERR) && fd_polls[i].fd == pipe_stderr[0] && stderr_active)
            {
                if ((bytes_read_err = read(pipe_stderr[0], buff_err, sizeof(buff_err) - 1)) > 0)
                {
                    buff_err[bytes_read_err] = '\0';
                    if (*live_print) { write(STDOUT_FILENO, buff_err, bytes_read_err); }
                    err.append(buff_err);
                }
                else
                {
                    fd_polls[i].fd = -1;
                    stderr_active = false; 
                }
            }
        }
    }

    close(pipe_stdout[0]);
    close(pipe_stderr[0]);
    if (stdin_active) { close(pipe_stdin[1]); }

    waitpid(pid, &statusWait, 0);

    if (WIFEXITED(statusWait))
    {
        return (PRESULT){WEXITSTATUS(statusWait), 0, out, err};
    }
    else if (WIFSIGNALED(statusWait))
    {
        int sig = WTERMSIG(statusWait);

        return (PRESULT){-1, sig, out, err};
    }
    else
    {
        return (PRESULT){-1, WSTOPSIG(statusWait), out, err};
    }
}

inline PRESULT execute_pty(std::vector<std::string> execution_argv, std::string binary_path, std::optional<char *const*> environment = static_cast<char *const*>(environ), std::optional<bool> live_print = true)
{
    std::vector<char*> argv;

    for (const auto& arg : execution_argv)
    {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    int master = -1;
    int slave = -1;

    posix_spawn_file_actions_t action;
    posix_spawnattr_t attribute;

    master = posix_openpt(O_RDWR | O_NOCTTY);

    if (master == -1)
    {
        return (PRESULT){1, 0, "", "Could not start virtual terminal device\n"};
    }

    if (grantpt(master) == -1)
    {
        return (PRESULT){1, 0, "", "Failed to grant permission to terminal device\n"};
    }

    if (unlockpt(master) == -1)
    {
        return (PRESULT){1, 0, "", "Failed to unlock slave endpoint\n"};
    }

    char *slave_name = ptsname(master);

    if (slave_name == nullptr)
    {
        return (PRESULT){1, 0, "", "Failed to retrieve slave path"};
    }

    posix_spawn_file_actions_init(&action);
    posix_spawnattr_init(&attribute);

    posix_spawn_file_actions_addopen(&action, STDIN_FILENO, slave_name, O_RDWR, 0);
    posix_spawn_file_actions_addopen(&action, STDOUT_FILENO, slave_name, O_RDWR, 0);
    posix_spawn_file_actions_addopen(&action, STDERR_FILENO, slave_name, O_RDWR, 0);

    posix_spawn_file_actions_addclose(&action, master);

    posix_spawn_file_actions_addclose(&action, slave);

    posix_spawnattr_setflags(&attribute, POSIX_SPAWN_SETSID);

    pid_t pid;
    int statusCode;

    try
    {
        statusCode = posix_spawn(&pid, binary_path.c_str(), &action, &attribute, argv.data(), *environment);
    }
    catch (std::bad_optional_access& e)
    {
        posix_spawn_file_actions_destroy(&action);
        posix_spawnattr_destroy(&attribute);
        close(master);
        return (PRESULT){1, 0, "", e.what()};
    }

    posix_spawn_file_actions_destroy(&action);
    posix_spawnattr_destroy(&attribute);

    close(slave);
    slave = -1;

    if (statusCode != 0)
    {
        return (PRESULT){statusCode, 0, "", std::strerror(statusCode)};
    }

    struct pollfd fd_polls[2];

    fd_polls[0].fd = master;
    fd_polls[0].events = POLLIN;
    fd_polls[1].fd = STDIN_FILENO;
    fd_polls[1].events = POLLIN;

    std::string out;
    std::string err;
    char buff_pty[1024];
    char buff_in[1024];
    ssize_t bytes_read_pty;
    ssize_t bytes_read_in;

    struct termios old_terminal;

    bool is_interactive = isatty(STDIN_FILENO);
    if (is_interactive)
    {
        tcgetattr(STDIN_FILENO, &old_terminal);
        struct termios raw_terminal = old_terminal;
        raw_terminal.c_lflag &= ~(ECHO | ICANON);
        raw_terminal.c_iflag |= ICRNL;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_terminal);
    }

    bool pty_active = true;

    while (pty_active)
    {
        poll(fd_polls, 2, -1);

        if (fd_polls[1].revents & POLLIN)
        {
            bytes_read_in = read(STDIN_FILENO, buff_in, sizeof(buff_in));
            if (bytes_read_in > 0)
            {
                write(master, buff_in, bytes_read_in);
            }
        }

        if (fd_polls[0].revents & (POLLIN | POLLHUP | POLLERR))
        {
            bytes_read_pty = read(master, buff_pty, sizeof(buff_pty) - 1);
            if (bytes_read_pty > 0)
            {
                buff_pty[bytes_read_pty] = '\0';
                if (*live_print) { write(STDOUT_FILENO, buff_pty, bytes_read_pty); }
                out.append(buff_pty);
            }

            else { pty_active = false; }
        }
    }

    if (is_interactive)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
    }
    
    close(master);

    int statusWait;

    waitpid(pid, &statusWait, 0);

    if (WIFEXITED(statusWait))
    {
        return (PRESULT){WEXITSTATUS(statusWait), 0, out, err};
    }
    else if (WIFSIGNALED(statusWait))
    {
        int sig = WTERMSIG(statusWait);

        return (PRESULT){-1, sig, out, err};
    }
    else
    {
        return (PRESULT){-1, WSTOPSIG(statusWait), out, err};
    }
}

}

#endif //PSCXX_HPP