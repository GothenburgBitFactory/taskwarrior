////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <Context.h>
#include <Column.h>
#include <Task.h>
#include <ViewTask.h>
#include <ViewText.h>
#include <test.h>
#include <main.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (2);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  try
  {
    // Set up configuration.
    context.config.setDefaults ();
    context.config.set ("fontunderline", true);
    context.config.set ("tag.indicator", "+");
    context.config.set ("dependency.indicator", "D");
    context.config.set ("recurrence.indicator", "R");
    context.config.set ("active.indicator", "A");
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
    std::string report = "view.t";
    ViewTask view;
    view.add (Column::factory ("id", report));
    view.add (Column::factory ("uuid.short", report));
    view.add (Column::factory ("project", report));
    view.add (Column::factory ("priority", report));
    view.add (Column::factory ("tags", report));
    view.add (Column::factory ("tags.count", report));
    view.add (Column::factory ("description", report));
    view.add (Column::factory ("depends.indicator", report));
    view.add (Column::factory ("recur.indicator", report));
    view.add (Column::factory ("status.short", report));
    view.add (Column::factory ("due.countdown", report));
    view.add (Column::factory ("start.active", report));
    view.add (Column::factory ("urgency", report));
    view.width (context.getWidth ());
    view.leftMargin (4);
    view.extraPadding (0);
    view.intraPadding (1);
    view.colorHeader (header_color);
    view.colorOdd (odd_color);
    view.colorEven (even_color);
    view.intraColorOdd (odd_color);
    view.intraColorEven (even_color);

    // Render the view.
    std::cout << view.render (data, sequence);
    int expected_lines = 5;
    if (!isatty (fileno (stdout)))
      expected_lines = 6;

    t.is (view.lines (), expected_lines, "View::lines == 5");

    // Now render a string-only grid.
    context.config.set ("fontunderline", false);
    Color single_cell ("bold white on red");

    ViewText string_view;
    string_view.width (context.getWidth ());
    string_view.leftMargin (4);
    string_view.extraPadding (0);
    string_view.intraPadding (1);
    string_view.colorHeader (header_color);
    string_view.colorOdd (odd_color);
    string_view.colorEven (even_color);
    string_view.intraColorOdd (odd_color);
    string_view.intraColorEven (even_color);

    string_view.add (Column::factory ("string", "One"));
    string_view.add (Column::factory ("string", "Two"));
    string_view.add (Column::factory ("string", "Three"));

    int row = string_view.addRow ();
    string_view.set (row, 0, "top left");
    string_view.set (row, 1, "top center");
    string_view.set (row, 2, "top right");

    row = string_view.addRow ();
    string_view.set (row, 0, "bottom left", single_cell);
    string_view.set (row, 1, "bottom center, containing sufficient text that "
                             "wrapping will occur because it exceeds all "
                             "reasonable values for default width.  Even in a "
                             "very wide terminal window.  Just look at the "
                             "lengths we must go to, to get passing unit tests "
                             "and not flaky tests.");
    string_view.set (row, 2, "bottom right");

    std::cout << string_view.render ();
    t.ok (string_view.lines () > 4, "View::lines > 4");
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
