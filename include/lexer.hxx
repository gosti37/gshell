#ifndef LEXER_HXX
#define LEXER_HXX

#include <string>
#include <vector>

enum tokenKind
{
    TOKEN_WORD,
    TOKEN_VAR
};

struct Token
{
  tokenKind kind;
  std::string str;
};

std::vector<std::string> vectorise(std::string str);
Token nextToken(std::string input);

#endif //LEXER_HXX