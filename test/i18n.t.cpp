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
#include <string>
#include <unistd.h>
#include <stdarg.h>

#include <Context.h>
#include <text.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (11);

  try
  {
    t.is (format ("pre {1} post",    "mid"),        "pre mid post",    "format 1a");
    t.is (format ("pre {1}{1} post", "mid"),        "pre midmid post", "format 1b");
    t.is (format ("pre {1} post",    0),            "pre 0 post",      "format 1c");
    t.is (format ("pre {1}{1} post", 0),            "pre 00 post",     "format 1d");

    t.is (format ("pre {1}{2} post", "one", "two"), "pre onetwo post", "format 2a");
    t.is (format ("pre {2}{1} post", "one", "two"), "pre twoone post", "format 2b");
    t.is (format ("pre {1}{2} post", "one", 2),     "pre one2 post",   "format 2c");
    t.is (format ("pre {1}{2} post", 1, "two"),     "pre 1two post",   "format 2d");
    t.is (format ("pre {1}{2} post", 1, 2),         "pre 12 post",     "format 2e");

    t.is (format ("pre {1}{2}{3} post", "one", "two", "three"), "pre onetwothree post", "format 3a");
    t.is (format ("pre {3}{1}{2} post", "one", "two", "three"), "pre threeonetwo post", "format 3b");
  }

  catch (std::string& error)
  {
    t.diag (error);
    return -1;
  }

  catch (...)
  {
    t.diag ("Unknown error.");
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
