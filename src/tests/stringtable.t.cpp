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
#include <unistd.h>
#include <StringTable.h>
#include <util.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (4);

  // Create a string file.
  std::string file = "./strings.xx-XX";
  spit (file, "# comment\n1 found");
  t.is (access (file.c_str (), F_OK), 0, "strings.xx-XX created.");

  // Load the string file.
  StringTable st;
  st.load (file);

  // Test the object.
  t.is (st.get (1, "nope"), "found", "string 1 'found' found");
  t.is (st.get (2, "nope"), "nope",  "string 2 'nope'  defaulted");

  // Clean up.
  unlink (file.c_str ());
  t.is (access (file.c_str (), F_OK), -1, "strings.xx-XX removed.");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
