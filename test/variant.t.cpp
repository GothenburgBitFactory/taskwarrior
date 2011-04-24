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
#include <Context.h>
#include <Variant.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (6);

  try
  {
    Variant v = Variant (1) + Variant (2);
    t.ok (v.type () == Variant::v_integer, "1 + 2 --> integer");
    t.ok (v.format () == "3", "1 + 2 --> 3");

    v = Variant (1.2) + Variant (2.3);
    t.ok (v.type () == Variant::v_double, "1.2 + 2.3 --> double");
    t.ok (v.format () == "3.5", "1.2 + 2.3 --> 3.5");

    v = Variant (1.2) + Variant (2);
    t.ok (v.type () == Variant::v_double, "1.2 + 2 --> double");
    t.ok (v.format () == "3.2", "1.2 + 2 --> 3.2");
  }

  catch (std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
