#ifndef PSCXX_HPP
#define PSCXX_HPP

#include <cstring>
#include <optional>
#include <spawn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <poll.h>
#include <string>
#include <vector>

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

    inline PRESULT execute_pip(std::vector<std::string> execution_argv, std::string binary_path, std::optional<char *const*> environment = static_cast<char *const*>(environ), std::optional<bool> live_print = true, std::optional<int> std_out = STDOUT_FILENO, std::optional<int> std_err = STDERR_FILENO, std::optional<int> std_in = STDIN_FILENO)
    {
        std::vector<char*> argv;

        for (const auto& arg : execution_argv)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        int status;
        int pipe_stdout[2];
        int pipe_stderr[2];
        int pipe_stdin[2];

        pid_t pid;
        posix_spawn_file_actions_t actions;

        status = posix_spawn_file_actions_init(&actions);
        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }

        status = pipe(pipe_stdout);
        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }
        status = pipe(pipe_stderr);
        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }
        status = pipe(pipe_stdin);
        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }

        try
        {
            posix_spawn_file_actions_adddup2(&actions, pipe_stdout[1], STDOUT_FILENO);
            posix_spawn_file_actions_adddup2(&actions, pipe_stderr[1], STDERR_FILENO);
            posix_spawn_file_actions_adddup2(&actions, pipe_stdin[0], STDIN_FILENO);
        }
        catch (std::bad_optional_access& e)
        {
            return (PRESULT){1, 0, "", e.what()};
        }

        for (int i = 0; i < 2; i++)
        {
            posix_spawn_file_actions_addclose(&actions, pipe_stdout[i]);
            posix_spawn_file_actions_addclose(&actions, pipe_stderr[i]);
            posix_spawn_file_actions_addclose(&actions, pipe_stdin[i]);
        }
        
        try
        {
            status = posix_spawn(&pid, binary_path.c_str(), &actions, nullptr, argv.data(), *environment);
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

        if (status != 0)
        { 
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            close(pipe_stdin[0]);
            close(pipe_stdout[1]);
            close(pipe_stderr[1]);
            close(pipe_stdin[1]);
            return (PRESULT){status, 0, "", std::strerror(status)};
        }

        posix_spawn_file_actions_destroy(&actions);
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);
        close(pipe_stdin[0]);

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
        fd_polls[2].fd = *std_in;

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
                if (fd_polls[i].revents & (POLLIN | POLLHUP | POLLERR) && fd_polls[i].fd == *std_in && stdin_active)
                {
                    bytes_read_in = read(*std_in, buff_in, sizeof(buff_in));
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
                        write(*std_out, buff_out, bytes_read_out);
                        out.append(buff_out, bytes_read_out);
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
                        if (*live_print) { write(STDERR_FILENO, buff_err, bytes_read_err); }
                        write(*std_err, buff_err, bytes_read_err);
                        err.append(buff_err, bytes_read_err);
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

        int status_wait;

        waitpid(pid, &status_wait, 0);

        if (WIFEXITED(status_wait))
        {
            return (PRESULT){WEXITSTATUS(status_wait), 0, out, err};
        }
        else if (WIFSIGNALED(status_wait))
        {
            int sig = WTERMSIG(status_wait);

            return (PRESULT){-1, sig, out, err};
        }
        else
        {
            return (PRESULT){-1, WSTOPSIG(status_wait), out, err};
        }
    }

    inline PRESULT execute_int(std::vector<std::string> execution_argv, std::string binary_path, std::optional<char *const*> environment = static_cast<char *const*>(environ))
    {
        std::vector<char*> argv;

        for (const auto& arg : execution_argv)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        pid_t pid;
        int status;
        posix_spawnattr_t attributes;

        status = posix_spawnattr_init(&attributes);
        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }

        posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP);
        posix_spawnattr_setpgroup(&attributes, 0);

        try
        {
            status = posix_spawn(&pid, binary_path.c_str(), nullptr, &attributes, argv.data(), *environment);
        }
        catch (std::bad_optional_access& e)
        {
            return (PRESULT){1, 0, "", e.what()};
        }

        posix_spawnattr_destroy(&attributes);

        if (status != 0) { return (PRESULT){status, 0, "", std::strerror(status)}; }

        int set_result = tcsetpgrp(STDIN_FILENO, pid);
        if (set_result < 0) { return (PRESULT){1, 0, "", std::strerror(errno)}; }

        int status_wait;
        waitpid(pid, &status_wait, 0);

        set_result = tcsetpgrp(STDIN_FILENO, getpgrp());
        if (set_result < 0) { return (PRESULT){1, 0, "", std::strerror(errno)}; }

        if (WIFEXITED(status_wait))
        {
            return (PRESULT){WEXITSTATUS(status_wait), 0, "", ""};
        }
        else if (WIFSIGNALED(status_wait))
        {
            int sig = WTERMSIG(status_wait);

            return (PRESULT){-1, sig, "", ""};
        }
        else
        {
            return (PRESULT){-1, WSTOPSIG(status_wait), "", ""};
        }
    }
}
#endif //PSCXX_HPP