#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <fstream>
#include <unistd.h>
#include <pwd.h>
#include "env.hxx"
#include "parser.hxx"
#include "tfmt.hpp"

std::unordered_map<std::string, std::string> environmentVars;

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

std::string getPaths(void)
{
    const char *paths_env = std::getenv("PATH");
    std::string paths(paths_env);
    return paths;
}

int envInit(void)
{
    environmentVars["USER"] = getUserName();
    auto userVar = environmentVars.find("USER");
    std::string defUser = userVar->second;

    environmentVars["HOME"] = "/home/" + defUser;
    auto homeVar = environmentVars.find("HOME");
    std::string defHome = homeVar->second;

    environmentVars["DEF_CONF"] = defHome + "/.gshconf";
    auto confVar = environmentVars.find("DEF_CONF");
    std::string defPath = confVar->second;

    std::string defBin(getPaths());
    environmentVars["DEF_BIN"] = defBin;

    std::ifstream confFileDef(defPath);

    if (!confFileDef)
    {
        std::ofstream confFileCreate(defPath);

        if (!confFileCreate)
        {
            tfmt::print(stderr, "Default configuration file could not be created: {}\n", std::strerror(errno));
            return errno;
        }

        confFileCreate.close();

        envInit();
    }

    std::string confLine;

    while (getline(confFileDef, confLine))
    {
        parse(confLine);
    }

    confFileDef.close();
    return 0;
}