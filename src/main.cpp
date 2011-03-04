////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>

#ifdef CYGWIN
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "Context.h"
#include "../cmake.h"

Context context;

int main (int argc, char** argv)
{
  // Set up randomness.
#ifdef CYGWIN
  #ifdef HAVE_SRANDOM
  srandom (time (NULL));
  #else
  srand (time (NULL));
  #endif
#else
  #ifdef HAVE_SRANDOM
  struct timeval tv;
  gettimeofday (&tv, NULL);
  srandom (tv.tv_usec);
  #else
  srand (tv.tv_usec);
  #endif
#endif

  int status = 0;

  try
  {
    context.initialize (argc, argv);

/* From 2.0.0
    std::string::size_type task = context.program.find ("/task");
    std::string::size_type t    = context.program.find ("/t");
    std::string::size_type cal  = context.program.find ("/cal");

    if (context.program != "task"                                            &&
        context.program != "t"                                               &&
        context.program != "cal"                                             &&
        (task == std::string::npos || context.program.length () != task + 5) &&
        (t    == std::string::npos || context.program.length () != t    + 2) &&
        (cal  == std::string::npos || context.program.length () != cal  + 4))
      status = context.handleInteractive ();
    else
      status = context.run ();
*/

    std::string::size_type itask = context.program.find ("/itask");
    if (context.program == "itask" ||
        (itask != std::string::npos && context.program.length () == itask + 5))
      status = context.interactive ();
    else
      status = context.run ();
  }

  catch (std::string& error)
  {
    std::cout << error << "\n";
    return -1;
  }

  catch (...)
  {
    std::cerr << context.stringtable.get (100, "Unknown error.") << "\n";
    return -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
