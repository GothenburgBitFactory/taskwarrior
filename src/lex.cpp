////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Lexer.h>
#include <Context.h>

Context context;

int main (int argc, char** argv)
{
  for (auto i = 1; i < argc; i++)
  {
    std::cout << "argument '" << argv[i] << "'\n";

    auto all = Lexer::tokens (argv[i]);
    for (auto token : Lexer::tokens (argv[i]))
      std::cout << "  token '" << token.first << "' " << Lexer::typeToString (token.second) << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
