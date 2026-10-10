#include <fstream>
#include "linenoise.h"
#include "tfmt.hpp"
#include "env.hxx"
#include "parser.hxx"

int main(int argc, char **argv)
{
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
        while (1)
        {
            char *input_raw = linenoise("GShell > ");

            if (!input_raw || static_cast<std::string>(input_raw) == "quit") break;

            std::string input(input_raw);

            if (input.empty()) continue;

            linenoiseFree(input_raw);

            parse(input);
        }
    }
    return 0;
}