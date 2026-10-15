#include <vector>
#include <string>
#include <spawn.h>
#include <sys/wait.h>
#include "builtins.hpp"
#include "env.hpp"
#include "tfmt.hpp"
#include "pscxx.hpp"



void execute(std::vector<std::string> exec_input)
{
    if (exec_input.at(0) == "right") { right(); return;}
    else if (exec_input.at(0) == "left") { left(); return;}
    else if (exec_input.at(0) == "say") { say(exec_input); return;}
    else if (exec_input.at(0) == "sad") { sad(); return;}
    else if (exec_input.at(0) == "sdir") { sdir(exec_input); return;}
    else if (exec_input.at(0) == "look") { look(exec_input); return;}
    else if (exec_input.at(0) == "set") { set(exec_input); return;}
    else if (exec_input.at(0) == "enable") { enable(exec_input); return;}
    else if (exec_input.at(0) == "showenv") { showenv(); return;}
    
    for (auto const& path : defPaths)
    {
        std::string exec_path = path + '/' + exec_input.at(0);

        pscxx::PRESULT result = pscxx::execute_pty(exec_input, exec_path, exportedVars.data());

        if (result.exitCode == 2)
        {
            continue;
        }

        if (result.exitCode != 0)
        {
            tfmt::print(stderr, "returned with {}\n", result.exitCode);
            return;
        }

        if (result.signal != 0)
        {
            tfmt::print(stderr, "Program terminated with signal {}\n", result.signal);
        }

        return;
    }

    tfmt::print(stderr, "gshell : Command not found = {}\n", exec_input.at(0));
}