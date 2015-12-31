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
#include <CmdHistory.h>
#include <sstream>
#include <Context.h>
#include <Filter.h>
#include <ViewText.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <ISO8601.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdHistoryMonthly::CmdHistoryMonthly ()
{
  _keyword               = "history.monthly";
  _usage                 = "task <filter> history.monthly";
  _description           = STRING_CMD_HISTORY_USAGE_M;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHistoryMonthly::execute (std::string& output)
{
  int rc = 0;

  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  for (auto& task : filtered)
  {
    ISO8601d entry (task.get_date ("entry"));

    ISO8601d end;
    if (task.has ("end"))
      end = ISO8601d (task.get_date ("end"));

    time_t epoch = entry.startOfMonth ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus () != Task::recurring)
      ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus () == Task::deleted)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus () == Task::completed)
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

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int priorYear = 0;
  int row = 0;
  for (auto& i : groups)
  {
    row = view.addRow ();

    totalAdded     += addedGroup     [i.first];
    totalCompleted += completedGroup [i.first];
    totalDeleted   += deletedGroup   [i.first];

    ISO8601d dt (i.first);
    int m, d, y;
    dt.toMDY (m, d, y);

    if (y != priorYear)
    {
      view.set (row, 0, y);
      priorYear = y;
    }
    view.set (row, 1, ISO8601d::monthName(m));

    int net = 0;

    if (addedGroup.find (i.first) != addedGroup.end ())
    {
      view.set (row, 2, addedGroup[i.first]);
      net +=addedGroup[i.first];
    }

    if (completedGroup.find (i.first) != completedGroup.end ())
    {
      view.set (row, 3, completedGroup[i.first]);
      net -= completedGroup[i.first];
    }

    if (deletedGroup.find (i.first) != deletedGroup.end ())
    {
      view.set (row, 4, deletedGroup[i.first]);
      net -= deletedGroup[i.first];
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
  _keyword               = "history.annual";
  _usage                 = "task <filter> history.annual";
  _description           = STRING_CMD_HISTORY_USAGE_A;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHistoryAnnual::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  for (auto& task : filtered)
  {
    ISO8601d entry (task.get_date ("entry"));

    ISO8601d end;
    if (task.has ("end"))
      end = ISO8601d (task.get_date ("end"));

    time_t epoch = entry.startOfYear ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus () != Task::recurring)
      ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus () == Task::deleted)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus () == Task::completed)
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

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int priorYear = 0;
  int row = 0;
  for (auto& i : groups)
  {
    row = view.addRow ();

    totalAdded     += addedGroup     [i.first];
    totalCompleted += completedGroup [i.first];
    totalDeleted   += deletedGroup   [i.first];

    ISO8601d dt (i.first);
    int m, d, y;
    dt.toMDY (m, d, y);

    if (y != priorYear)
    {
      view.set (row, 0, y);
      priorYear = y;
    }

    int net = 0;

    if (addedGroup.find (i.first) != addedGroup.end ())
    {
      view.set (row, 1, addedGroup[i.first]);
      net +=addedGroup[i.first];
    }

    if (completedGroup.find (i.first) != completedGroup.end ())
    {
      view.set (row, 2, completedGroup[i.first]);
      net -= completedGroup[i.first];
    }

    if (deletedGroup.find (i.first) != deletedGroup.end ())
    {
      view.set (row, 3, deletedGroup[i.first]);
      net -= deletedGroup[i.first];
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
  _keyword               = "ghistory.monthly";
  _usage                 = "task <filter> ghistory.monthly";
  _description           = STRING_CMD_GHISTORY_USAGE_M;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdGHistoryMonthly::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  for (auto& task : filtered)
  {
    ISO8601d entry (task.get_date ("entry"));

    ISO8601d end;
    if (task.has ("end"))
      end = ISO8601d (task.get_date ("end"));

    time_t epoch = entry.startOfMonth ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus () != Task::recurring)
      ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus () == Task::deleted)
    {
      epoch = end.startOfMonth ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus () == Task::completed)
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
  view.add (Column::factory ("string",            STRING_CMD_GHISTORY_YEAR));
  view.add (Column::factory ("string",            STRING_CMD_GHISTORY_MONTH));
  view.add (Column::factory ("string.left_fixed", STRING_CMD_GHISTORY_NUMBER));

  Color color_add    (context.config.get ("color.history.add"));
  Color color_done   (context.config.get ("color.history.done"));
  Color color_delete (context.config.get ("color.history.delete"));
  Color label        (context.config.get ("color.label"));

  view.colorHeader (label);

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  for (auto& i : groups)
  {
    if (completedGroup[i.first] + deletedGroup[i.first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i.first] + deletedGroup[i.first];

    if (addedGroup[i.first] > maxAddedLine)
      maxAddedLine = addedGroup[i.first];
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
    for (auto& i : groups)
    {
      row = view.addRow ();

      totalAdded     += addedGroup[i.first];
      totalCompleted += completedGroup[i.first];
      totalDeleted   += deletedGroup[i.first];

      ISO8601d dt (i.first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        view.set (row, 0, y);
        priorYear = y;
      }
      view.set (row, 1, ISO8601d::monthName(m));

      unsigned int addedBar     = (widthOfBar *     addedGroup[i.first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i.first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i.first]) / maxLine;

      std::string bar = "";
      if (context.color ())
      {
        std::string aBar = "";
        if (addedGroup[i.first])
        {
          aBar = format (addedGroup[i.first]);
          while (aBar.length () < addedBar)
            aBar = " " + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i.first])
        {
          cBar = format (completedGroup[i.first]);
          while (cBar.length () < completedBar)
            cBar = " " + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i.first])
        {
          dBar = format (deletedGroup[i.first]);
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
                     color_done.colorize (STRING_CMD_HISTORY_COMP),
                     color_delete.colorize (STRING_CMD_HISTORY_DEL))
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
  _keyword               = "ghistory.annual";
  _usage                 = "task <filter> ghistory.annual";
  _description           = STRING_CMD_GHISTORY_USAGE_A;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdGHistoryAnnual::execute (std::string& output)
{
  int rc = 0;
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  for (auto& task : filtered)
  {
    ISO8601d entry (task.get_date ("entry"));

    ISO8601d end;
    if (task.has ("end"))
      end = ISO8601d (task.get_date ("end"));

    time_t epoch = entry.startOfYear ().toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus () != Task::recurring)
      ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus () == Task::deleted)
    {
      epoch = end.startOfYear ().toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus () == Task::completed)
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
  view.add (Column::factory ("string",            STRING_CMD_GHISTORY_YEAR));
  view.add (Column::factory ("string.left_fixed", STRING_CMD_GHISTORY_NUMBER));

  Color color_add    (context.config.get ("color.history.add"));
  Color color_done   (context.config.get ("color.history.done"));
  Color color_delete (context.config.get ("color.history.delete"));
  Color label        (context.config.get ("color.label"));

  view.colorHeader (label);

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  for (auto& i : groups)
  {
    if (completedGroup[i.first] + deletedGroup[i.first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i.first] + deletedGroup[i.first];

    if (addedGroup[i.first] > maxAddedLine)
      maxAddedLine = addedGroup[i.first];
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
    for (auto& i : groups)
    {
      row = view.addRow ();

      totalAdded     += addedGroup[i.first];
      totalCompleted += completedGroup[i.first];
      totalDeleted   += deletedGroup[i.first];

      ISO8601d dt (i.first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        view.set (row, 0, y);
        priorYear = y;
      }

      unsigned int addedBar     = (widthOfBar *     addedGroup[i.first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i.first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i.first]) / maxLine;

      std::string bar = "";
      if (context.color ())
      {
        std::string aBar = "";
        if (addedGroup[i.first])
        {
          aBar = format (addedGroup[i.first]);
          while (aBar.length () < addedBar)
            aBar = " " + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i.first])
        {
          cBar = format (completedGroup[i.first]);
          while (cBar.length () < completedBar)
            cBar = " " + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i.first])
        {
          dBar = format (deletedGroup[i.first]);
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
                     color_done.colorize (STRING_CMD_HISTORY_COMP),
                     color_delete.colorize (STRING_CMD_HISTORY_DEL))
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
