#include <vector>
#include <string>
#include <filesystem>
#include <spawn.h>
#include <sys/wait.h>
#include "builtins.hxx"
#include "env.hxx"
#include "tfmt.hpp"

void executeBinary(std::vector<std::string> bin_paths, std::vector<std::string> bin_input)
{
    int count = 0;
    pid_t pid;

    for (const auto& path : bin_paths)
    {
        try
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                for (const auto& item : std::filesystem::directory_iterator(path))
                {
                    if (count == bin_paths.size())
                    {
                        tfmt::print(stderr, "gshell : command not found = {}\n", bin_input.at(0));
                        return;
                    }
                    if (item.path().filename().string() == bin_input.at(0))
                    {
                        std::vector<std::string> environmentProperties;
                        environmentProperties.reserve(environmentVars.size() + 1);
                        for (auto const& item : environmentVars)
                        {
                            environmentProperties.push_back(item.first + "=" + item.second);
                        }

                        std::vector<char*> env;

                        env.reserve(environmentVars.size());
                        for (auto& str : environmentProperties)
                        {
                            env.push_back(str.data());
                        }
                        env.push_back(nullptr);

                        std::vector<char*> argv;
                        for (auto const& arg : bin_input)
                        {
                            argv.push_back(const_cast<char*>(arg.c_str()));
                        }
                        argv.push_back(nullptr);

                        int spawnRes = posix_spawn(&pid, std::filesystem::absolute(item.path().string()).c_str(), nullptr, nullptr, argv.data(), env.data());

                        if (spawnRes != 0)
                        {
                            std::error_code ec(spawnRes, std::generic_category());
                            tfmt::print(stderr, "Process start failed = {} ({})\n", ec.message(), ec.value());
                            return;
                        }
                        else
                        {
                            int wait_stat;
                            if (waitpid(pid, &wait_stat, 0) == -1)
                            {
                                tfmt::print(stderr, "Process with ID {} failed waitpid\n", pid);
                                return;
                            }
                            else
                            {
                                if(WIFEXITED(wait_stat))
                                {
                                    int exit_code = WIFEXITED(wait_stat);

                                    if (exit_code == 127)
                                    {
                                        tfmt::print(stderr, "Child process failed to execute binary\ns");
                                    }

                                    return;
                                }
                            }
                        }
                    }
                    else if (!std::filesystem::exists(std::filesystem::absolute(item.path().string())))
                    {
                        count++;
                        continue;
                    }
                }
            }
        }
        catch (std::filesystem::filesystem_error& e)
        {
            tfmt::print(stderr, "Filesystem error: {}\n", e.what());
            return;
        }
    }
}

void execute(std::vector<std::string> exec_input)
{
    if (exec_input.at(0) == "right") { right(); return;}
    else if (exec_input.at(0) == "left") { left(); return;}
    else if (exec_input.at(0) == "say") { say(exec_input); return;}
    else if (exec_input.at(0) == "sad") { sad(); return;}
    else if (exec_input.at(0) == "sdir") { sdir(exec_input); return;}
    else if (exec_input.at(0) == "look") { look(exec_input); return;}
    else if (exec_input.at(0) == "set") { set(exec_input); return;}
    else
    {
        std::vector<std::string> paths;
        std::string path_line;

        auto command = environmentVars.find("DEF_BIN");
        if (command != environmentVars.end())
        {
            std::stringstream ss(command->second);
            while (getline(ss, path_line, ':'))
            {
                paths.push_back(path_line);
            }

            executeBinary(paths, exec_input);
        }

        return;
    }
}