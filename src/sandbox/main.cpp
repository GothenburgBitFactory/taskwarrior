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
//    c.run ();

////////////////////////////////////////////////////////////////////////////////

    c.tdb.lock (c.config.get ("locking", true));

    c.filter.push_back (Att ("priority", "L"));

    std::vector <T2> tasks;
    int quantity = c.tdb.load (tasks, c.filter);
    std::cout << "# " << quantity << " <-- c.tdb.load" << std::endl;

    c.tdb.unlock ();

////////////////////////////////////////////////////////////////////////////////

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
