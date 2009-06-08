////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include "Context.h"

Context context;

int main (int argc, char** argv)
{
  try
  {
    context.initialize (argc, argv);
    context.tdb.lock (context.config.get ("locking", true));

    context.filter.push_back (Att ("priority", "L"));

    std::vector <T2> tasks;
    int quantity = context.tdb.load (tasks, context.filter);
    std::cout << "# " << quantity << " <-- context.tdb.load" << std::endl;

    context.tdb.unlock ();
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
