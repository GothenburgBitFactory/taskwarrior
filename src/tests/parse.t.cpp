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
#include <iostream>
#include "task.h"
#include "text.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (18);

  std::vector <std::string> args;
  std::string command;

  Config conf;
  conf.set ("dateformat", "m/d/Y");

  {
    T task;
    split (args, "add foo", ' ');
    parse (args, command, task, conf);
    t.is (command,                "add", "(1) command found");
    t.is (task.getId (),          0,     "(1) zero id on add");
    t.is (task.getDescription (), "foo", "(1) correct description");
  }

  {
    T task;
    split (args, "delete 1,3-5,7", ' ');
    parse (args, command, task, conf);
    std::vector <int> sequence = task.getAllIds ();
    t.is (sequence.size (), (size_t)5, "(2) sequence length");
    if (sequence.size () == 5)
    {
      t.is (sequence[0], 1, "(2) sequence[0] == 1");
      t.is (sequence[1], 3, "(2) sequence[1] == 3");
      t.is (sequence[2], 4, "(2) sequence[2] == 4");
      t.is (sequence[3], 5, "(2) sequence[3] == 5");
      t.is (sequence[4], 7, "(2) sequence[4] == 7");
    }
    else
    {
      t.fail ("(2) sequence[0] == 1");
      t.fail ("(2) sequence[1] == 3");
      t.fail ("(2) sequence[2] == 4");
      t.fail ("(2) sequence[3] == 5");
      t.fail ("(2) sequence[4] == 7");
    }
  }

  {
    T task;
    split (args, "delete 1,2 3,4", ' ');
    parse (args, command, task, conf);
    std::vector <int> sequence = task.getAllIds ();
    t.is (sequence.size (), (size_t)4, "(3) sequence length");
    if (sequence.size () == 4)
    {
      t.is (sequence[0], 1, "(3) sequence[0] == 1");
      t.is (sequence[1], 2, "(3) sequence[1] == 2");
      t.is (sequence[2], 3, "(3) sequence[2] == 3");
      t.is (sequence[3], 4, "(3) sequence[3] == 4");
    }
    else
    {
      t.fail ("(3) sequence[0] == 1");
      t.fail ("(3) sequence[1] == 2");
      t.fail ("(3) sequence[2] == 3");
      t.fail ("(3) sequence[3] == 4");
    }
  }

  {
    T task;
    split (args, "1 There are 7 days in a week", ' ');
    parse (args, command, task, conf);
    std::vector <int> sequence = task.getAllIds ();
    t.is (sequence.size (), (size_t)1, "(4) sequence length");
    if (sequence.size () == 1)
    {
      t.is (sequence[0], 1, "(4) sequence[0] == 1");
    }
    else
    {
      t.fail ("(4) sequence[0] == 1");
    }
  }

  {
    T task;
    args.clear ();
    args.push_back ("1");
    args.push_back ("4-123 is back-ordered");
    parse (args, command, task, conf);
    std::vector <int> sequence = task.getAllIds ();
    t.is (sequence.size (), (size_t)1, "(5) sequence length");
    if (sequence.size () == 1)
    {
      t.is (sequence[0], 1, "(5) sequence[0] == 1");
    }
    else
    {
      t.fail ("(5) sequence[0] == 1");
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

