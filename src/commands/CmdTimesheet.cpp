////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <Date.h>
#include <main.h>
#include <i18n.h>
#include <text.h>
#include <CmdTimesheet.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdTimesheet::CmdTimesheet ()
{
  _keyword     = "timesheet";
  _usage       = "task          timesheet [weeks]";
  _description = STRING_CMD_TIMESHEET_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdTimesheet::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks.
  handleRecurrence ();
  std::vector <Task> all = context.tdb2.all_tasks ();

  // What day of the week does the user consider the first?
  int weekStart = Date::dayOfWeek (context.config.get ("weekstart"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string (STRING_DATE_BAD_WEEKSTART);

  // Determine the date of the first day of the most recent report.
  Date today;
  Date start;
  start -= (((today.dayOfWeek () - weekStart) + 7) % 7) * 86400;

  // Roll back to midnight.
  start = Date (start.month (), start.day (), start.year ());
  Date end = start + (7 * 86400);

  // Determine how many reports to run.
  int quantity = 1;
  std::vector <std::string> words = context.cli.getWords ();
  if (words.size () == 1)
    quantity = strtol (words[0].c_str (), NULL, 10);;

  std::stringstream out;
  for (int week = 0; week < quantity; ++week)
  {
    Date endString (end);
    endString -= 86400;

    std::string title = start.toString (context.config.get ("dateformat"))
                        + " - "
                        + endString.toString (context.config.get ("dateformat"));

    Color bold (Color::nocolor, Color::nocolor, false, true, false);
    out << "\n"
        << (context.color () ? bold.colorize (title) : title)
        << "\n";

    // Render the completed table.
    ViewText completed;
    completed.width (context.getWidth ());
    completed.add (Column::factory ("string",       "   "));
    completed.add (Column::factory ("string",       STRING_COLUMN_LABEL_PROJECT));
    completed.add (Column::factory ("string.right", STRING_COLUMN_LABEL_DUE));
    completed.add (Column::factory ("string",       STRING_COLUMN_LABEL_DESC));

    Color label (context.config.get ("color.label"));
    completed.colorHeader (label);

    for (auto& task : all)
    {
      // If task completed within range.
      if (task.getStatus () == Task::completed)
      {
        Date compDate (task.get_date ("end"));
        if (compDate >= start && compDate < end)
        {
          Color c;
          if (context.color ())
            autoColorize (task, c);

          int row = completed.addRow ();
          std::string format = context.config.get ("dateformat.report");
          if (format == "")
            format = context.config.get ("dateformat");
          completed.set (row, 1, task.get ("project"), c);

          if(task.has ("due"))
          {
            Date dt (task.get_date ("due"));
            completed.set (row, 2, dt.toString (format));
          }

          std::string description = task.get ("description");
          int indent = context.config.getInteger ("indent.annotation");

          std::map <std::string, std::string> annotations;
          task.getAnnotations (annotations);
          for (auto& ann : annotations)
            description += "\n"
                         + std::string (indent, ' ')
                         + Date (ann.first.substr (11)).toString (context.config.get ("dateformat"))
                         + " "
                         + ann.second;

          completed.set (row, 3, description, c);
        }
      }
    }

    out << "  " << format (STRING_CMD_TIMESHEET_DONE, completed.rows ()) << "\n";

    if (completed.rows ())
      out << completed.render ()
          << "\n";

    // Now render the started table.
    ViewText started;
    started.width (context.getWidth ());
    started.add (Column::factory ("string",       "   "));
    started.add (Column::factory ("string",       STRING_COLUMN_LABEL_PROJECT));
    started.add (Column::factory ("string.right", STRING_COLUMN_LABEL_DUE));
    started.add (Column::factory ("string",       STRING_COLUMN_LABEL_DESC));
    started.colorHeader (label);

    for (auto& task : all)
    {
      // If task started within range, but not completed withing range.
      if (task.getStatus () == Task::pending &&
          task.has ("start"))
      {
        Date startDate (task.get_date ("start"));
        if (startDate >= start && startDate < end)
        {
          Color c;
          if (context.color ())
            autoColorize (task, c);

          int row = started.addRow ();
          std::string format = context.config.get ("dateformat.report");
          if (format == "")
            format = context.config.get ("dateformat");
          started.set (row, 1, task.get ("project"), c);

          if (task.has ("due"))
          {
            Date dt (task.get_date ("due"));
            started.set (row, 2, dt.toString (format));
          }

          std::string description = task.get ("description");
          int indent = context.config.getInteger ("indent.annotation");

          std::map <std::string, std::string> annotations;
          task.getAnnotations (annotations);
          for (auto& ann : annotations)
            description += "\n"
                         + std::string (indent, ' ')
                         + Date (ann.first.substr (11)).toString (context.config.get ("dateformat"))
                         + " "
                         + ann.second;

          started.set (row, 3, description, c);
        }
      }
    }

    out << "  " << format (STRING_CMD_TIMESHEET_STARTED, started.rows ()) << "\n";

    if (started.rows ())
      out << started.render ()
          << "\n\n";

    // Prior week.
    start -= 7 * 86400;
    end   -= 7 * 86400;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
