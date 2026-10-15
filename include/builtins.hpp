#ifndef BUILTINS_HXX
#define BUILTINS_HXX

#include <vector>
#include <string>

int right(void);
int left(void);
int say(std::vector<std::string>& input);
int sad(void);
int sdir(std::vector<std::string>& input);
int look(std::vector<std::string>& input);
int set(std::vector<std::string>& input);
int enable(std::vector<std::string>& input);
int showenv(void);

#endif //BUILTINS_HXX