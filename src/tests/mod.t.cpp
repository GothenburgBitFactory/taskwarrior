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
#include <Mod.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (18);

  Mod m = "before";     t.ok (m.valid (), "Mod: before is valid");
      m = "after";      t.ok (m.valid (), "Mod: after is valid");
      m = "not";        t.ok (m.valid (), "Mod: not is valid");
      m = "none";       t.ok (m.valid (), "Mod: none is valid");
      m = "any";        t.ok (m.valid (), "Mod: any is valid");
      m = "over";       t.ok (m.valid (), "Mod: over is valid");
      m = "under";      t.ok (m.valid (), "Mod: under is valid");
      m = "synth";      t.ok (m.valid (), "Mod: synth is valid");
      m = "first";      t.ok (m.valid (), "Mod: first is valid");
      m = "last";       t.ok (m.valid (), "Mod: last is valid");
      m = "this";       t.ok (m.valid (), "Mod: this is valid");
      m = "next";       t.ok (m.valid (), "Mod: next is valid");
      m = "is";         t.ok (m.valid (), "Mod: is is valid");
      m = "isnt";       t.ok (m.valid (), "Mod: isnt is valid");
      m = "has";        t.ok (m.valid (), "Mod: has is valid");
      m = "hasnt";      t.ok (m.valid (), "Mod: hasnt is valid");
      m = "startswith"; t.ok (m.valid (), "Mod: startswith is valid");
      m = "endswith";   t.ok (m.valid (), "Mod: endswith is valid");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
