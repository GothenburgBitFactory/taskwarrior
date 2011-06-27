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

#define L10N                                           // Localization complete.

#include <sstream>
#include <Context.h>
#include <ViewText.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <CmdHistory.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdHistoryMonthly::CmdHistoryMonthly ()
{
  _keyword     = "history.monthly";
  _usage       = "task history.monthly [<filter>]";
  _description = STRING_CMD_HISTORY_USAGE_M;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHistoryMonthly::execute (std::string& output)
{
  int rc = 0;

  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Date entry (task->get ("entry"));

    Date end;
    if (task->has ("end"))
      end = Date (task->get ("end"));

    time_t epoch = entry.startOfMonth ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date.
    ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  // Now build the view.
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string",       STRING_CMD_HISTORY_YEAR));
  view.add (Column::factory ("string",       STRING_CMD_HISTORY_MONTH));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_ADDED));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_COMP));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_DEL));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_NET));

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int priorYear = 0;
  int row = 0;
  std::map <time_t, int>::iterator i;
  for (i = groups.begin (); i != groups.end (); ++i)
  {
    row = view.addRow ();

    totalAdded     += addedGroup     [i->first];
    totalCompleted += completedGroup [i->first];
    totalDeleted   += deletedGroup   [i->first];

    Date dt (i->first);
    int m, d, y;
    dt.toMDY (m, d, y);

    if (y != priorYear)
    {
      view.set (row, 0, y);
      priorYear = y;
    }
    view.set (row, 1, Date::monthName(m));

    int net = 0;

    if (addedGroup.find (i->first) != addedGroup.end ())
    {
      view.set (row, 2, addedGroup[i->first]);
      net +=addedGroup[i->first];
    }

    if (completedGroup.find (i->first) != completedGroup.end ())
    {
      view.set (row, 3, completedGroup[i->first]);
      net -= completedGroup[i->first];
    }

    if (deletedGroup.find (i->first) != deletedGroup.end ())
    {
      view.set (row, 4, deletedGroup[i->first]);
      net -= deletedGroup[i->first];
    }

    Color net_color;
    if (context.color () && net)
      net_color = net > 0
                    ? Color (Color::red)
                    : Color (Color::green);

    view.set (row, 5, net, net_color);
  }

  if (view.rows ())
  {
    row = view.addRow ();
    view.set (row, 0, " ");
    row = view.addRow ();

    Color row_color;
    if (context.color ())
      row_color = Color (Color::nocolor, Color::nocolor, false, true, false);

    view.set (row, 1, STRING_CMD_HISTORY_AVERAGE, row_color);
    view.set (row, 2, totalAdded     / (view.rows () - 2), row_color);
    view.set (row, 3, totalCompleted / (view.rows () - 2), row_color);
    view.set (row, 4, totalDeleted   / (view.rows () - 2), row_color);
    view.set (row, 5, (totalAdded - totalCompleted - totalDeleted) / (view.rows () - 2), row_color);
  }

  std::stringstream out;
  if (view.rows ())
    out << optionalBlankLine ()
        << view.render ()
        << "\n";
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdHistoryAnnual::CmdHistoryAnnual ()
{
  _keyword     = "history.annual";
  _usage       = "task history.annual [<filter>]";
  _description = STRING_CMD_HISTORY_USAGE_A;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHistoryAnnual::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Date entry (task->get ("entry"));

    Date end;
    if (task->has ("end"))
      end = Date (task->get ("end"));

    time_t epoch = entry.startOfYear ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date.
    ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  // Now build the view.
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string",       STRING_CMD_HISTORY_YEAR));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_ADDED));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_COMP));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_DEL));
  view.add (Column::factory ("string.right", STRING_CMD_HISTORY_NET));

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int priorYear = 0;
  int row = 0;
  std::map <time_t, int>::iterator i;
  for (i = groups.begin (); i != groups.end (); ++i)
  {
    row = view.addRow ();

    totalAdded     += addedGroup     [i->first];
    totalCompleted += completedGroup [i->first];
    totalDeleted   += deletedGroup   [i->first];

    Date dt (i->first);
    int m, d, y;
    dt.toMDY (m, d, y);

    if (y != priorYear)
    {
      view.set (row, 0, y);
      priorYear = y;
    }

    int net = 0;

    if (addedGroup.find (i->first) != addedGroup.end ())
    {
      view.set (row, 1, addedGroup[i->first]);
      net +=addedGroup[i->first];
    }

    if (completedGroup.find (i->first) != completedGroup.end ())
    {
      view.set (row, 2, completedGroup[i->first]);
      net -= completedGroup[i->first];
    }

    if (deletedGroup.find (i->first) != deletedGroup.end ())
    {
      view.set (row, 3, deletedGroup[i->first]);
      net -= deletedGroup[i->first];
    }

    Color net_color;
    if (context.color () && net)
      net_color = net > 0
                    ? Color (Color::red)
                    : Color (Color::green);

    view.set (row, 4, net, net_color);
  }

  if (view.rows ())
  {
    view.addRow ();
    row = view.addRow ();

    Color row_color;
    if (context.color ())
      row_color = Color (Color::nocolor, Color::nocolor, false, true, false);

    view.set (row, 0, STRING_CMD_HISTORY_AVERAGE, row_color);
    view.set (row, 1, totalAdded     / (view.rows () - 2), row_color);
    view.set (row, 2, totalCompleted / (view.rows () - 2), row_color);
    view.set (row, 3, totalDeleted   / (view.rows () - 2), row_color);
    view.set (row, 4, (totalAdded - totalCompleted - totalDeleted) / (view.rows () - 2), row_color);
  }

  std::stringstream out;
  if (view.rows ())
    out << optionalBlankLine ()
        << view.render ()
        << "\n";
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdGHistoryMonthly::CmdGHistoryMonthly ()
{
  _keyword     = "ghistory.monthly";
  _usage       = "task ghistory.monthly [<filter>]";
  _description = STRING_CMD_GHISTORY_USAGE_M;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdGHistoryMonthly::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Date entry (task->get ("entry"));

    Date end;
    if (task->has ("end"))
      end = Date (task->get ("end"));

    time_t epoch = entry.startOfMonth ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date.
    ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  int widthOfBar = context.getWidth () - 15;   // 15 == strlen ("2008 September ")

  // Now build the view.
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string", STRING_CMD_GHISTORY_YEAR));
  view.add (Column::factory ("string", STRING_CMD_GHISTORY_MONTH));
  view.add (Column::factory ("string", STRING_CMD_GHISTORY_NUMBER));

  Color color_add    (context.config.get ("color.history.add"));
  Color color_done   (context.config.get ("color.history.done"));
  Color color_delete (context.config.get ("color.history.delete"));

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  std::map <time_t, int>::iterator i;
  for (i = groups.begin (); i != groups.end (); ++i)
  {
    if (completedGroup[i->first] + deletedGroup[i->first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i->first] + deletedGroup[i->first];

    if (addedGroup[i->first] > maxAddedLine)
      maxAddedLine = addedGroup[i->first];
  }

  int maxLine = maxAddedLine + maxRemovedLine;
  if (maxLine > 0)
  {
    unsigned int leftOffset = (widthOfBar * maxAddedLine) / maxLine;

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    int priorYear = 0;
    int row = 0;
    std::map <time_t, int>::iterator i;
    for (i = groups.begin (); i != groups.end (); ++i)
    {
      row = view.addRow ();

      totalAdded     += addedGroup[i->first];
      totalCompleted += completedGroup[i->first];
      totalDeleted   += deletedGroup[i->first];

      Date dt (i->first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        view.set (row, 0, y);
        priorYear = y;
      }
      view.set (row, 1, Date::monthName(m));

      unsigned int addedBar     = (widthOfBar *     addedGroup[i->first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i->first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i->first]) / maxLine;

      std::string bar = "";
      if (context.color ())
      {
        std::string aBar = "";
        if (addedGroup[i->first])
        {
          aBar = format (addedGroup[i->first]);
          while (aBar.length () < addedBar)
            aBar = " " + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i->first])
        {
          cBar = format (completedGroup[i->first]);
          while (cBar.length () < completedBar)
            cBar = " " + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i->first])
        {
          dBar = format (deletedGroup[i->first]);
          while (dBar.length () < deletedBar)
            dBar = " " + dBar;
        }

        bar += std::string (leftOffset - aBar.length (), ' ');

        bar += color_add.colorize    (aBar);
        bar += color_done.colorize   (cBar);
        bar += color_delete.colorize (dBar);
      }
      else
      {
        std::string aBar = ""; while (aBar.length () < addedBar)     aBar += "+";
        std::string cBar = ""; while (cBar.length () < completedBar) cBar += "X";
        std::string dBar = ""; while (dBar.length () < deletedBar)   dBar += "-";

        bar += std::string (leftOffset - aBar.length (), ' ');
        bar += aBar + cBar + dBar;
      }

      view.set (row, 2, bar);
    }
  }

  std::stringstream out;
  if (view.rows ())
  {
    out << optionalBlankLine ()
        << view.render ()
        << "\n";

    if (context.color ())
      out << format (STRING_CMD_HISTORY_LEGEND,
                     color_add.colorize (STRING_CMD_HISTORY_ADDED),
                     color_add.colorize (STRING_CMD_HISTORY_COMP),
                     color_add.colorize (STRING_CMD_HISTORY_DEL))
          << optionalBlankLine ()
          << "\n";
    else
      out << STRING_CMD_HISTORY_LEGEND_A
          << "\n";
  }
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdGHistoryAnnual::CmdGHistoryAnnual ()
{
  _keyword     = "ghistory.annual";
  _usage       = "task ghistory.annual [<filter>]";
  _description = STRING_CMD_GHISTORY_USAGE_A;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdGHistoryAnnual::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Date entry (task->get ("entry"));

    Date end;
    if (task->has ("end"))
      end = Date (task->get ("end"));

    time_t epoch = entry.startOfYear ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date.
    ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  int widthOfBar = context.getWidth () - 5;   // 5 == strlen ("YYYY ")

  // Now build the view.
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string", STRING_CMD_GHISTORY_YEAR));
  view.add (Column::factory ("string", STRING_CMD_GHISTORY_NUMBER));

  Color color_add    (context.config.get ("color.history.add"));
  Color color_done   (context.config.get ("color.history.done"));
  Color color_delete (context.config.get ("color.history.delete"));

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  std::map <time_t, int>::iterator i;
  for (i = groups.begin (); i != groups.end (); ++i)
  {
    if (completedGroup[i->first] + deletedGroup[i->first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i->first] + deletedGroup[i->first];

    if (addedGroup[i->first] > maxAddedLine)
      maxAddedLine = addedGroup[i->first];
  }

  int maxLine = maxAddedLine + maxRemovedLine;
  if (maxLine > 0)
  {
    unsigned int leftOffset = (widthOfBar * maxAddedLine) / maxLine;

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    int priorYear = 0;
    int row = 0;
    std::map <time_t, int>::iterator i;
    for (i = groups.begin (); i != groups.end (); ++i)
    {
      row = view.addRow ();

      totalAdded     += addedGroup[i->first];
      totalCompleted += completedGroup[i->first];
      totalDeleted   += deletedGroup[i->first];

      Date dt (i->first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        view.set (row, 0, y);
        priorYear = y;
      }

      unsigned int addedBar     = (widthOfBar *     addedGroup[i->first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i->first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i->first]) / maxLine;

      std::string bar = "";
      if (context.color ())
      {
        std::string aBar = "";
        if (addedGroup[i->first])
        {
          aBar = format (addedGroup[i->first]);
          while (aBar.length () < addedBar)
            aBar = " " + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i->first])
        {
          cBar = format (completedGroup[i->first]);
          while (cBar.length () < completedBar)
            cBar = " " + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i->first])
        {
          dBar = format (deletedGroup[i->first]);
          while (dBar.length () < deletedBar)
            dBar = " " + dBar;
        }

        bar += std::string (leftOffset - aBar.length (), ' ');
        bar += color_add.colorize    (aBar);
        bar += color_done.colorize   (cBar);
        bar += color_delete.colorize (dBar);
      }
      else
      {
        std::string aBar = ""; while (aBar.length () < addedBar)     aBar += "+";
        std::string cBar = ""; while (cBar.length () < completedBar) cBar += "X";
        std::string dBar = ""; while (dBar.length () < deletedBar)   dBar += "-";

        bar += std::string (leftOffset - aBar.length (), ' ');
        bar += aBar + cBar + dBar;
      }

      view.set (row, 1, bar);
    }
  }

  std::stringstream out;
  if (view.rows ())
  {
    out << optionalBlankLine ()
        << view.render ()
        << "\n";

    if (context.color ())
      out << format (STRING_CMD_HISTORY_LEGEND,
                     color_add.colorize (STRING_CMD_HISTORY_ADDED),
                     color_add.colorize (STRING_CMD_HISTORY_COMP),
                     color_add.colorize (STRING_CMD_HISTORY_DEL))
          << optionalBlankLine ()
          << "\n";
    else
      out << STRING_CMD_HISTORY_LEGEND_A
          << "\n";
  }
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
