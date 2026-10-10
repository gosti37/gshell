#include <cerrno>
#include <cstdio>
#include <cstring>
#include <system_error>
#include <vector>
#include <filesystem>
#include <fstream>
#include "builtins.hxx"
#include "env.hxx"
#include "tfmt.hpp"

using namespace std;

int right(void)
{
    return 0;
}

int left(void)
{
    return 1;
}

int say(vector<string>& input)
{
    bool flag_e{false};
    bool flag_E{false};
    bool flag_n{false};

    for (size_t i = 1; i < input.size(); i++)
    {
        if (input.at(i).at(0) == '-' && input.at(i).size() > 1)
        {
            switch (input.at(i).at(1))
            {
                case ('e'):
                flag_e = true;
                break;
                case ('E'):
                flag_E = true;
                break;
                case ('n'):
                flag_n = true;
                break;
                default:
                tfmt::print(stderr, "Invalid flag");
                return 1;
            }
        }

        else
        {
            if (flag_e)
            {
                for (size_t c = 0; c < input.at(i).size(); c++)
                {
                    if (input.at(i).at(c) == '\\' && c + 1 < input.at(i).size())
                    {
                        switch (input.at(i).at(c + 1))
                        {
                            case ('n'):
                            tfmt::print("\n");
                            c++;
                            break;
                            case ('t'):
                            tfmt::print("\t");
                            c++;
                            break;
                            case ('a'):
                            tfmt::print("\a");
                            c++;
                            break;
                            case ('b'):
                            tfmt::print("\b");
                            c++;
                            break;
                            case ('r'):
                            tfmt::print("\r");
                            c++;
                            break;
                            case ('\\'):
                            tfmt::print("\\");
                            c++;
                            break;
                        }
                    }
                    else
                    {
                        tfmt::print("{}", input.at(i).at(c));
                    }
                }
            }

            else if (flag_E)
            {
                for (size_t c = 0; c < input.at(i).size(); c++)
                {
                    tfmt::print("{}", input.at(i).at(c));
                }
            }

            else
            {
                tfmt::print("{}", input.at(i));
                if (i + 1 < input.size()) tfmt::print(" ");
            }
        }
    }

    if (!flag_n)
    {
        tfmt::print("\n");
    }

    return 0;
}

int sad(void)
{
    error_code ec;
    auto path = filesystem::current_path(ec);

    if (ec)
    {
        tfmt::print(stderr, "Error trying to look at the current directory: {}\n", ec.message());
        return 1;
    }

    tfmt::print("{}\n", path.string());
    return 0;
}

int sdir(vector<string>& input)
{
    if (input.size() < 2)
    {
        tfmt::print(stderr, "Enter a valid path to switch\n");
        return 1;
    }

    error_code ec;

    filesystem::current_path(input.at(1), ec);

    if (ec)
    {
        tfmt::print(stderr, "Error trying to switch to path '{}': {}\n", input.at(1), ec.message());
        return 1;
    }

    return 0;
}

int look(vector<string>& input)
{
    if (input.size() < 2)
    {
        tfmt::print(stderr, "Please enter a valid file to look at\n");
        return 1;
    }

    string text;
    ifstream file(input.at(1));

    if (!file)
    {
        tfmt::print(stderr, "Looking at file {} failed: {}\n", input.at(1), std::strerror(errno));
        return 1;
    }

    while (getline(file, text))
    {
        tfmt::print(stdout, "{}", text);
    }

    tfmt::print("\n");

    return 0;
}

int set(vector<string>& input)
{
    if (input.size() < 2)
    {
        tfmt::print(stderr, "Please enter a valid variable name and a value to it (USAGE: set $VARNAME$ <VARVALUE>)");
        return 1;
    }

    environmentVars[input.at(1)] = input.at(2);

    return 0;
}