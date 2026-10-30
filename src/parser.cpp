#include <filesystem>
#include <sstream>
#include "builtins.hpp"
#include "env.hpp"
#include "lexer.hpp"
#include "exec.hpp"
#include "tfmt.hpp"
#include "pscxx.hpp"

void parse(std::string& input)
{
    Token token;
    std::vector<std::string> argv;
    std::string str;
    std::stringstream ss(input);

    while (ss >> str)
    {
        argv.push_back(str);
    }

    for (size_t i = 0; i < argv.size(); i++)
    {
        token = nextToken(argv.at(i));

        if (token.kind == TOKEN_WORD) continue;
        if (token.kind == TOKEN_VAR)
        {
            auto find = environmentVars.find(token.str);
            if (find != environmentVars.end())
            {
                argv.at(i) = find->second.varVal;
                continue;
            }
            else
            {
                argv.at(i) = " ";
            }
        }
        if (token.kind == TOKEN_BIN)
        {
            if (std::filesystem::is_directory(argv.at(i)))
            {
                std::vector<std::string> vec = {" ", token.str};
                sdir(vec);
                return;
            }
            else
            {
                pscxx::PRESULT result = pscxx::execute_int(argv, token.str, exportedVars.data());

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
        }

            continue;
        }

    execute(argv);
    return;
}