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
    // Set up configuration.
    context.config.set ("fontunderline", true);

    // Two sample tasks.
    Task t1 ("[project:\"Home\"]");
    t1.id = 1;
    Task t2 ("[project:\"Garden Care\"]");
    t2.id = 11;

    std::vector <Task> data;
    data.push_back (t1);
    data.push_back (t2);

    // Sequence of tasks.
    std::vector <int> sequence;
    sequence.push_back (0);
    sequence.push_back (1);

    // Create colors.
    Color header_color (Color (Color::yellow, Color::nocolor, false, false, false));

    // Create a view.
    View view;
    view.add (Column::factory ("id"));
    view.add (Column::factory ("project"));
    view.width (16);
    view.leftMargin (4);
    view.extraPadding (0);
    view.intraPadding (1);
    view.colorHeader (header_color);

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
