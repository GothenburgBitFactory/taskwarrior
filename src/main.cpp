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

#include <i18n.h>
#include <Context.h>
#include <cmake.h>

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
    status = context.run ();
  }

  catch (std::string& error)
  {
    std::cout << error << "\n";
    return -1;
  }

  catch (...)
  {
    std::cerr << STRING_UNKNOWN_ERROR << "\n";
    return -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
