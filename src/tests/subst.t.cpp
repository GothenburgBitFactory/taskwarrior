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
#include <T.h>
#include <Subst.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (2);

  T task;
  task.set ("description", "one two three four");

  Subst s;
  if (s.parse ("/two/TWO/"))
  {
    s.apply (task);
    t.is (task.get ("description"), "one TWO three four", "single word subst");
  }
  else
  {
    t.fail ("failed to parse '/two/TWO/'");
  }

  if (s.parse ("/e /E /g"))
  {
    s.apply (task);
    t.is (task.get ("description"), "onE TWO threE four", "multiple word subst");
  }
  else
  {
    t.fail ("failed to parse '/e /E /g'");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
