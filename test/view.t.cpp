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
#include <Column.h>
#include <Task.h>
#include <View.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (1);

  try
  {
    // Two sample tasks.
    std::vector <Task> data;
    data.push_back (Task ("[description:\"Migrate import out of core\" entry:\"1303155011\" project:\"task-2.1\" status:\"pending\" uuid:\"3bb54f40-c38f-4936-aae6-f67de6227e00\"]"));
    data.push_back (Task ("[annotation_1303444800:\"Uli Martens\" description:\"New command: task show defaults, that dumps the Config.cpp defaults string, as an example of a complete .taskrc file\" entry:\"1303472714\" project:\"task-2.0\" status:\"pending\" uuid:\"f30cb9c3-3fc0-483f-bfb2-3bf134f00694\"]"));

    // Sequence of tasks.
    std::vector <int> sequence;
    sequence.push_back (0);
    sequence.push_back (1);

    // Create a view.
    View view;
    view.add (Column::factory ("id"));
    view.add (Column::factory ("project"));
    view.width (40);

    // Render the view.
    std::cout << view.render (data, sequence)
              << std::endl;
  }

  catch (std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
