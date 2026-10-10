#ifndef ENV_HXX
#define ENV_HXX

#include <string>
#include <unordered_map>

std::string getUserName(void);
int envInit(void);

extern std::unordered_map<std::string, std::string> environmentVars;

#endif //ENV_HXX