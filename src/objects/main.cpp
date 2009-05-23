////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include "Context.h"

int main (int argc, char** argv)
{
  try
  {
    Context c;
    c.initialize (argc, argv);
    c.run ();

    return 0;
  }

  catch (std::string e)
  {
    std::cerr << e << std::endl;
  }

  catch (...)
  {
    std::cerr << "task internal error." << std::endl;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
