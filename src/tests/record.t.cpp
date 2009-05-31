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
#include <Att.h>
#include <Record.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
Record parseRecord (const std::string& input)
{
  try
  {
    Record r (input);
    return r;
  }

  catch (std::string& e)
  {
    std::cout << "# Exception: " << e << std::endl;
  }

  catch (...)
  {
    std::cout << "# Exception!" << std::endl;
  }

  return Record ();
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (4);

  // (blank)
  Record record = parseRecord ("");
  t.is (record.size (), (size_t)0, "Record (blank)");

  // []
  record = parseRecord ("[]");
  t.is (record.size (), (size_t)0, "Record []");

  // [name:"value"]
  record = parseRecord ("[name:\"value\"]");
  t.is (record.size (), (size_t)1, "Record [name:value]");
  if (record.size () == 1)
  {
    Att a = record["name"];
    t.is (a.name (), "name", "Record [name:value] -> 'name'");
  }
  else
  {
    t.fail ("Record [name:value] -> 'name'");
  }

  // TODO [name:"value"]
  // TODO [name:"one two"]
  // TODO [one:two three:four]

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
