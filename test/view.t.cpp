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
#include <main.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (1);

  try
  {
    // Set up configuration.
    context.config.setDefaults ();
    context.config.set ("fontunderline", true);
    context.config.set ("tag.indicator", "+");
    context.config.set ("dependency.indicator", "D");
    context.config.set ("recurrence.indicator", "R");
    context.config.set ("dateformat", "Y-M-D");
    context.config.set ("indent.annotation", "2");

    // Two sample tasks.
    Task t1 ("["
               "status:\"pending\" "
               "uuid:\"2a64f6e0-bf8e-430d-bf71-9ec3f0d9b661\" "
               "description:\"This is the description text\" "
               "project:\"Home\" "
               "priority:\"H\" "
               "annotation_1234567890:\"This is an annotation\" "
               "start:\"1234567890\" "
               "due:\"1234567890\" "
               "tags:\"one,two\""
             "]");
    t1.id = 1;
    Task t2 ("["
               "status:\"pending\" "
               "uuid:\"f30cb9c3-3fc0-483f-bfb2-3bf134f00694\" "
               "description:\"This is the description text\" "
               "project:\"Garden Care\" "
               "recur:\"monthly\" "
               "depends:\"2a64f6e0-bf8e-430d-bf71-9ec3f0d9b661\""
             "]");
    t2.id = 11;
    Task t3 ("["
               "status:\"pending\" "
               "uuid:\"c44cb9c3-3fc0-483f-bfb2-3bf134f05554\" "
               "description:\"Another description\" "
               "project:\"Garden\" "
             "]");
    t3.id = 8;

    std::vector <Task> data;
    data.push_back (t1);
    data.push_back (t2);
    data.push_back (t3);

    // Sequence of tasks.
    std::vector <int> sequence;
    sequence.push_back (0);
    sequence.push_back (1);
    sequence.push_back (2);

    sort_tasks (data, sequence, "description+,id-");

    // Create colors.
    Color header_color (Color (Color::yellow, Color::nocolor, false, false, false));
    Color odd_color ("on gray1");
    Color even_color ("on gray0");

    // Create a view.
    View view;
    view.add (Column::factory ("id"));
    view.add (Column::factory ("uuid.short"));
    view.add (Column::factory ("project"));
    view.add (Column::factory ("priority.long"));
    view.add (Column::factory ("tags"));
//    view.add (Column::factory ("tags.indicator"));
    view.add (Column::factory ("tags.count"));
    view.add (Column::factory ("description"));
//    view.add (Column::factory ("description.desc"));
//    view.add (Column::factory ("description.truncated"));
//    view.add (Column::factory ("description.oneline"));
//    view.add (Column::factory ("description.count"));
//    view.add (Column::factory ("depends"));
//    view.add (Column::factory ("depends.count"));
    view.add (Column::factory ("depends.indicator"));
//    view.add (Column::factory ("recur"));
    view.add (Column::factory ("recur.indicator"));
//    view.add (Column::factory ("status"));
    view.add (Column::factory ("status.short"));
//    view.add (Column::factory ("due"));
//    view.add (Column::factory ("due.julian"));
    view.add (Column::factory ("due.countdown"));
//    view.add (Column::factory ("due.epoch"));
//    view.add (Column::factory ("due.iso"));
    view.add (Column::factory ("start.active"));
    view.add (Column::factory ("urgency"));
    view.width (context.getWidth ());
    view.leftMargin (4);
/*
    view.extraPadding (1);
*/
    view.extraPadding (0);
    view.intraPadding (1);
    view.colorHeader (header_color);
    view.colorOdd (odd_color);
    view.colorEven (even_color);
    view.intraColorOdd (odd_color);
    view.intraColorEven (even_color);
/*
    view.extraColorOdd (odd_color);
    view.extraColorEven (even_color);
*/

    // Render the view.
    std::cout << view.render (data, sequence);

    t.is (view.lines (), 5, "View::lines == 5");
  }

  catch (std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
