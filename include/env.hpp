#ifndef ENV_HXX
#define ENV_HXX

#include <string>
#include <vector>
#include <unordered_map>

struct Variable
{
    std::string varKey;
    std::string varVal;
    bool is_exported;
};

std::string getUserName(void);
void envInit(void);

extern std::unordered_map<std::string, Variable> environmentVars;
extern std::vector<char*> exportedVars;
extern std::vector<std::string> defPaths;

#endif //ENV_HXX
