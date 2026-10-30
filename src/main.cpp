#include <csignal>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include "editline.h"
#include "tfmt.hpp"
#include "env.hpp"
#include "parser.hpp"

int main(int argc, char **argv)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    envInit();

    if (argc > 1)
    {
        std::string suffix = ".gsh";
        std::string filename(argv[1]);
        if ((filename.compare(filename.size() - suffix.size(), suffix.length(), suffix)) == 0)
        {
            std::ifstream file(filename);
            std::string file_line;

            while (getline(file, file_line))
            {
                parse(file_line);
            }

            file.close();
        }

        else
        {
            tfmt::print(stderr,"File {} is not valid, enter a valid file (Usage: gshell <FILENAME.gsh>)", filename);
            return 1;
        }
    }

    else
    {
        std::string name = getUserName();
        std::string home = "/home/" + name;
        std::string path;
        char hname[HOST_NAME_MAX];
        gethostname(hname, HOST_NAME_MAX);
        std::string prompt;

        while (1)
        {
            path = std::string(std::filesystem::current_path().c_str());
            if (path.find(home) != std::string::npos)
            {
                prompt = name + '@' + hname + "->" + '~' + path.substr(home.length(), path.npos) + "> ";
            }
            else
            {
                prompt = name + '@' + hname + "->" + path + "> ";
            }

            std::string input;

            if (char *input_raw = ::readline(prompt.c_str()); input_raw == nullptr) {
                tfmt::print("\n");
                break;
            }
            else {
                input = input_raw;
                ::free(input_raw);
            }

            if (input.empty()) continue;
            if (input == "exit") break;
            parse(input);
        }
    }
}