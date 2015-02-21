////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Lexer2.h>
#include <Context.h>

Context context;

int main (int argc, char** argv)
{
  for (auto i = 1; i < argc; i++)
  {
    std::cout << "input '" << argv[i] << "'\n";
    // Low-level tokens.
    Lexer2 lexer (argv[i]);
    std::string token;
    Lexer2::Type type;
    while (lexer.token (token, type))
      std::cout << "  token '" << token << "' " << Lexer2::typeToString (type) << "\n";

/*
    // High-level tokens.
    auto all = Lexer2::tokens (argv[i]);
    for (auto token : Lexer2::tokens (argv[i]))
      std::cout << "  token '" << token.first << "' " << Lexer2::typeToString (token.second) << "\n";
*/
  }
}

////////////////////////////////////////////////////////////////////////////////
