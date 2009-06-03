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
#include <Context.h>
#include <T2.h>
#include <Subst.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (3);

  T2 task;
  task.set ("description", "one two three four");

  Subst s;
  if (s.parse ("/two/TWO/"))
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "one TWO three four", "single word subst");
  }
  else
  {
    t.fail ("failed to parse '/two/TWO/'");
  }

  if (s.parse ("/e /E /g"))
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "onE two threE four", "multiple word subst");
  }
  else
  {
    t.fail ("failed to parse '/e /E /g'");
  }

  if (s.parse ("/from/to/g"))
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "one two three four", "multiple word subst mismatch");
  }
  else
  {
    t.fail ("failed to parse '/from/to/g'");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
