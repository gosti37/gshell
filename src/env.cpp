#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <pwd.h>
#include "env.hpp"
#include "parser.hpp"
#include "tfmt.hpp"

extern char **environ;

std::unordered_map<std::string, Variable> environmentVars;
std::vector<char*> exportedVars;
std::vector<std::string> defPaths;

std::string getUserName(void)
{
    uid_t uid = getuid();

    struct passwd* pw = getpwuid(uid);

    if (pw && pw->pw_name)
    {
        return std::string(pw->pw_name);
    }

    return " ";
}

void getEnv(void)
{
    for (int i = 0; environ[i] != NULL; i++)
    {
        std::string var = environ[i];
        ssize_t del_pos = var.find("=");

        if (del_pos != std::string::npos)
        {
            std::string key = var.substr(0, del_pos);
            std::string val = var.substr(del_pos + 1);

            environmentVars[key] = {.varKey = key, .varVal = val, .is_exported = true};
            exportedVars.push_back(environ[i]);
        }
    }

    exportedVars.push_back(nullptr);

    return;
}

void envInit(void)
{
    environmentVars.clear();
    exportedVars.clear();
    defPaths.clear();
    
    getEnv();

    std::string path_line;

    auto pathFind = environmentVars.find("PATH");

    if (pathFind != environmentVars.end())
    {
        std::istringstream iss(pathFind->second.varVal);

        while (getline(iss, path_line, ':'))
        {
            defPaths.push_back(path_line);
        }
    }
    else 
    {
        tfmt::print(stderr, "PATH could not be found\n");
    }

    std::string name = getUserName();

    std::ifstream confDefault("/home/" + name + "/.ghsconfig");

    if (!confDefault)
    {
        std::ofstream confCreate ("/home/" + name + "/.ghsconfig");

        if (!confCreate)
        {
            tfmt::print(stderr, "Could not create default config");
        }

        return;

    }

    std::string line;

    while (getline(confDefault, line))
    {
        parse(line);
    }

    confDefault.close();

    return;
}