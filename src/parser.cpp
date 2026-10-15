#include <sstream>
#include "env.hpp"
#include "lexer.hpp"
#include "exec.hpp"

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

            continue;
        }

    execute(argv);
    return;
}