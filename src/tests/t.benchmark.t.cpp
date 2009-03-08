////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <sys/time.h>
#include "../T.h"
#include "../task.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (1);

  std::string sample = "d346065c-7ef6-49af-ae77-19c1825807f5 "
                       "- "
                       "[bug performance solaris linux osx] "
                       "[due:1236142800 entry:1236177552 priority:H project:task-1.5.0 start:1236231761] "
                       "Profile task and identify performance bottlenecks";

  // Start clock
  test.diag ("start");
  struct timeval start;
  gettimeofday (&start, NULL);

  for (int i = 0; i < 1000000; i++)
  {
    T t (sample);
  }

  // End clock
  struct timeval end;
  gettimeofday (&end, NULL);
  test.diag ("end");

  int diff = ((end.tv_sec   * 1000000) + end.tv_usec) -
             ((start.tv_sec * 1000000) + start.tv_usec);

  char s[16];
  sprintf (s, "%d.%06d", diff/1000000, diff%1000000);
  test.pass (std::string ("1,000,000 T::parse calls in ") + s + "s");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

