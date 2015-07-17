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

    Lexer l (argv[i]);
    std::string token;
    Lexer::Type type;
    while (l.token (token, type))
      std::cout << "  token '" << token << "' " << Lexer::typeToString (type) << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
