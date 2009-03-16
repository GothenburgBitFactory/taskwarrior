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
#include "../T.h"
#include "../task.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (5);

  T t;
  std::string s = t.compose ();
  test.is ((int)s.length (), 46, "T::T (); T::compose ()");
  test.diag (s);

  t.setStatus (T::completed);
  s = t.compose ();
  test.is (s[37], '+', "T::setStatus (completed)");
  test.diag (s);

  t.setStatus (T::deleted);
  s = t.compose ();
  test.is (s[37], 'X', "T::setStatus (deleted)");
  test.diag (s);

  t.setStatus (T::recurring);
  s = t.compose ();
  test.is (s[37], 'r', "T::setStatus (recurring)");
  test.diag (s);

  // Round trip test.
  std::string sample = "00000000-0000-0000-0000-000000000000 - [] [] Sample";
  T t2;
  t2.parse (sample);
  sample += "\n";
  test.is (t2.compose (), sample, "T::parse -> T::compose round trip");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

