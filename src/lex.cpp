////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Lexer.h>
#include <Context.h>

Context context;

int main (int argc, char** argv)
{
  for (auto i = 1; i < argc; i++)
  {
    std::cout << "input '" << argv[i] << "'\n";
    // Low-level tokens.
    Lexer lexer (argv[i]);
    std::string token;
    Lexer::Type type;
    while (lexer.token (token, type))
      std::cout << "  token '" << token << "' " << Lexer::typeToString (type) << "\n";

/*
    // High-level tokens.
    auto all = Lexer::tokens (argv[i]);
    for (auto token : Lexer::tokens (argv[i]))
      std::cout << "  token '" << token.first << "' " << Lexer::typeToString (token.second) << "\n";
*/
  }
}

////////////////////////////////////////////////////////////////////////////////
