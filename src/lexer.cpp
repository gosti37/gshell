#include "lexer.hpp"
#include <filesystem>
#include <sstream>

using namespace std;

vector<string> vectorise(string input)
{
    vector<string> vec;
    stringstream ss(input);
    string word;

    while (ss >> word)
    {
        vec.push_back(word);
    }

    return vec;
}

Token nextToken(string input)
{
    vector<string> vecMain = vectorise(input);

    for (size_t i = 0; i < vecMain.size(); i++)
    {
      if (vecMain.at(i).at(0) == '$')
      {
        auto varStr = vecMain[i].substr(1, vecMain[i].length() - 1);

        return (Token){.kind = TOKEN_VAR, .str = varStr};
      }

      else if(std::filesystem::exists(vecMain.at(i)))
      {
        return (Token){.kind = TOKEN_BIN, .str = vecMain.at(i)};
      }

      else
      {
        return (Token){.kind = TOKEN_WORD, .str = vecMain[i]};
      }
    }
}