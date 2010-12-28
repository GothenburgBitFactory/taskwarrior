////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010, Paul Beckingham, Federico Hernandez, Federico Hernandez.
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

#include <fstream>
#include <unistd.h>
#include "Context.h"
#include "Sensor.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest ut (7);

  // Make sure there is no file.
  unlink ("./sensor.foo");

  // Create sensor for missing file.
  Sensor s;
  s.fileModification ("./sensor.foo");
  ut.ok (!s.changed (), "file not yet changed");

  // Create the file.
  std::ofstream one ("./sensor.foo", std::ios_base::out | std::ios_base::app);
  if (one.good ())
  {
    one << "touch\n";
    one.close ();
  }

  // Should register the change, so reset.
  ut.ok (s.changed (), "file changed");
  s.reset ();
  ut.ok (!s.changed (), "file not yet changed");

  // Wait a little, then modify the file.
  ut.diag ("sleep 2");
  sleep (2);
  std::ofstream two ("./sensor.foo", std::ios_base::out | std::ios_base::app);
  if (two.good ())
  {
    two << "touch\n";
    two.close ();
  }

  ut.ok (s.changed (), "file changed");
  ut.ok (s.changed (), "file still changed");
  s.reset ();
  ut.ok (!s.changed (), "file not changed again");

  unlink ("./sensor.foo");
  ut.ok (s.changed (), "file changed");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

