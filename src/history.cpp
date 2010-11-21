////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

//#include <iostream>
//#include <iomanip>
#include <sstream>
//#include <fstream>
//#include <sys/types.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <pwd.h>
//#include <time.h>

#include "Context.h"
//#include "Directory.h"
//#include "File.h"
//#include "Date.h"
//#include "Duration.h"
#include "Table.h"
#include "text.h"
#include "util.h"
#include "main.h"

//#ifdef HAVE_LIBNCURSES
//#include <ncurses.h>
//#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int handleReportHistoryMonthly (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-history-command"))
  {
    std::map <time_t, int> groups;          // Represents any month with data
    std::map <time_t, int> addedGroup;      // Additions by month
    std::map <time_t, int> completedGroup;  // Completions by month
    std::map <time_t, int> deletedGroup;    // Deletions by month

    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      Date entry (task->get ("entry"));

      Date end;
      if (task->has ("end"))
        end = Date (task->get ("end"));

      time_t epoch = entry.startOfMonth ().toEpoch ();
      groups[epoch] = 0;

      // Every task has an entry date.
      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      // All deleted tasks have an end date.
      if (task->getStatus () == Task::deleted)
      {
        epoch = end.startOfMonth ().toEpoch ();
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }

      // All completed tasks have an end date.
      else if (task->getStatus () == Task::completed)
      {
        epoch = end.startOfMonth ().toEpoch ();
        groups[epoch] = 0;

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }

    // Now build the table.
    Table table;
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Year");
    table.addColumn ("Month");
    table.addColumn ("Added");
    table.addColumn ("Completed");
    table.addColumn ("Deleted");
    table.addColumn ("Net");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
      table.setColumnUnderline (2);
      table.setColumnUnderline (3);
      table.setColumnUnderline (4);
      table.setColumnUnderline (5);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnJustification (2, Table::right);
    table.setColumnJustification (3, Table::right);
    table.setColumnJustification (4, Table::right);
    table.setColumnJustification (5, Table::right);

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    int priorYear = 0;
    int row = 0;
    foreach (i, groups)
    {
      row = table.addRow ();

      totalAdded     += addedGroup     [i->first];
      totalCompleted += completedGroup [i->first];
      totalDeleted   += deletedGroup   [i->first];

      Date dt (i->first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        table.addCell (row, 0, y);
        priorYear = y;
      }
      table.addCell (row, 1, Date::monthName(m));

      int net = 0;

      if (addedGroup.find (i->first) != addedGroup.end ())
      {
        table.addCell (row, 2, addedGroup[i->first]);
        net +=addedGroup[i->first];
      }

      if (completedGroup.find (i->first) != completedGroup.end ())
      {
        table.addCell (row, 3, completedGroup[i->first]);
        net -= completedGroup[i->first];
      }

      if (deletedGroup.find (i->first) != deletedGroup.end ())
      {
        table.addCell (row, 4, deletedGroup[i->first]);
        net -= deletedGroup[i->first];
      }

      table.addCell (row, 5, net);
      if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) && net)
        table.setCellColor (row, 5, net > 0 ? Color (Color::red) :
                                              Color (Color::green));
    }

    if (table.rowCount ())
    {
      table.addRow ();
      row = table.addRow ();

      table.addCell (row, 1, "Average");
      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        table.setRowColor (row, Color (Color::nocolor, Color::nocolor, false, true, false));
      table.addCell (row, 2, totalAdded     / (table.rowCount () - 2));
      table.addCell (row, 3, totalCompleted / (table.rowCount () - 2));
      table.addCell (row, 4, totalDeleted   / (table.rowCount () - 2));
      table.addCell (row, 5, (totalAdded - totalCompleted - totalDeleted) / (table.rowCount () - 2));
    }

    std::stringstream out;
    if (table.rowCount ())
      out << optionalBlankLine ()
          << table.render ()
          << "\n";
    else
    {
      out << "No tasks.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-history-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportHistoryAnnual (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-history-command"))
  {
    std::map <time_t, int> groups;          // Represents any month with data
    std::map <time_t, int> addedGroup;      // Additions by month
    std::map <time_t, int> completedGroup;  // Completions by month
    std::map <time_t, int> deletedGroup;    // Deletions by month

    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      Date entry (task->get ("entry"));

      Date end;
      if (task->has ("end"))
        end = Date (task->get ("end"));

      time_t epoch = entry.startOfYear ().toEpoch ();
      groups[epoch] = 0;

      // Every task has an entry date.
      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      // All deleted tasks have an end date.
      if (task->getStatus () == Task::deleted)
      {
        epoch = end.startOfYear ().toEpoch ();
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }

      // All completed tasks have an end date.
      else if (task->getStatus () == Task::completed)
      {
        epoch = end.startOfYear ().toEpoch ();
        groups[epoch] = 0;

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }

    // Now build the table.
    Table table;
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Year");
    table.addColumn ("Added");
    table.addColumn ("Completed");
    table.addColumn ("Deleted");
    table.addColumn ("Net");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
      table.setColumnUnderline (2);
      table.setColumnUnderline (3);
      table.setColumnUnderline (4);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnJustification (1, Table::right);
    table.setColumnJustification (2, Table::right);
    table.setColumnJustification (3, Table::right);
    table.setColumnJustification (4, Table::right);

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    int priorYear = 0;
    int row = 0;
    foreach (i, groups)
    {
      row = table.addRow ();

      totalAdded     += addedGroup     [i->first];
      totalCompleted += completedGroup [i->first];
      totalDeleted   += deletedGroup   [i->first];

      Date dt (i->first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        table.addCell (row, 0, y);
        priorYear = y;
      }

      int net = 0;

      if (addedGroup.find (i->first) != addedGroup.end ())
      {
        table.addCell (row, 1, addedGroup[i->first]);
        net +=addedGroup[i->first];
      }

      if (completedGroup.find (i->first) != completedGroup.end ())
      {
        table.addCell (row, 2, completedGroup[i->first]);
        net -= completedGroup[i->first];
      }

      if (deletedGroup.find (i->first) != deletedGroup.end ())
      {
        table.addCell (row, 3, deletedGroup[i->first]);
        net -= deletedGroup[i->first];
      }

      table.addCell (row, 4, net);
      if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) && net)
        table.setCellColor (row, 4, net > 0 ? Color (Color::red) :
                                              Color (Color::green));
    }

    if (table.rowCount ())
    {
      table.addRow ();
      row = table.addRow ();

      table.addCell (row, 0, "Average");
      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        table.setRowColor (row, Color (Color::nocolor, Color::nocolor, false, true, false));
      table.addCell (row, 1, totalAdded     / (table.rowCount () - 2));
      table.addCell (row, 2, totalCompleted / (table.rowCount () - 2));
      table.addCell (row, 3, totalDeleted   / (table.rowCount () - 2));
      table.addCell (row, 4, (totalAdded - totalCompleted - totalDeleted) / (table.rowCount () - 2));
    }

    std::stringstream out;
    if (table.rowCount ())
      out << optionalBlankLine ()
          << table.render ()
          << "\n";
    else
    {
      out << "No tasks.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-history-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportGHistoryMonthly (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-ghistory-command"))
  {
    std::map <time_t, int> groups;          // Represents any month with data
    std::map <time_t, int> addedGroup;      // Additions by month
    std::map <time_t, int> completedGroup;  // Completions by month
    std::map <time_t, int> deletedGroup;    // Deletions by month

    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      Date entry (task->get ("entry"));

      Date end;
      if (task->has ("end"))
        end = Date (task->get ("end"));

      time_t epoch = entry.startOfMonth ().toEpoch ();
      groups[epoch] = 0;

      // Every task has an entry date.
      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      // All deleted tasks have an end date.
      if (task->getStatus () == Task::deleted)
      {
        epoch = end.startOfMonth ().toEpoch ();
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }

      // All completed tasks have an end date.
      else if (task->getStatus () == Task::completed)
      {
        epoch = end.startOfMonth ().toEpoch ();
        groups[epoch] = 0;

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }

    int widthOfBar = context.getWidth () - 15;   // 15 == strlen ("2008 September ")

    // Now build the table.
    Table table;
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Year");
    table.addColumn ("Month");
    table.addColumn ("Number Added/Completed/Deleted");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }
    else
      table.setTableDashedUnderline ();

    Color color_add    (context.config.get ("color.history.add"));
    Color color_done   (context.config.get ("color.history.done"));
    Color color_delete (context.config.get ("color.history.delete"));

    // Determine the longest line, and the longest "added" line.
    int maxAddedLine = 0;
    int maxRemovedLine = 0;
    foreach (i, groups)
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
      foreach (i, groups)
      {
        row = table.addRow ();

        totalAdded     += addedGroup[i->first];
        totalCompleted += completedGroup[i->first];
        totalDeleted   += deletedGroup[i->first];

        Date dt (i->first);
        int m, d, y;
        dt.toMDY (m, d, y);

        if (y != priorYear)
        {
          table.addCell (row, 0, y);
          priorYear = y;
        }
        table.addCell (row, 1, Date::monthName(m));

        unsigned int addedBar     = (widthOfBar *     addedGroup[i->first]) / maxLine;
        unsigned int completedBar = (widthOfBar * completedGroup[i->first]) / maxLine;
        unsigned int deletedBar   = (widthOfBar *   deletedGroup[i->first]) / maxLine;

        std::string bar = "";
        if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        {
          char number[24];
          std::string aBar = "";
          if (addedGroup[i->first])
          {
            sprintf (number, "%d", addedGroup[i->first]);
            aBar = number;
            while (aBar.length () < addedBar)
              aBar = " " + aBar;
          }

          std::string cBar = "";
          if (completedGroup[i->first])
          {
            sprintf (number, "%d", completedGroup[i->first]);
            cBar = number;
            while (cBar.length () < completedBar)
              cBar = " " + cBar;
          }

          std::string dBar = "";
          if (deletedGroup[i->first])
          {
            sprintf (number, "%d", deletedGroup[i->first]);
            dBar = number;
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

        table.addCell (row, 2, bar);
      }
    }

    std::stringstream out;
    if (table.rowCount ())
    {
      out << optionalBlankLine ()
          << table.render ()
          << "\n";

      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        out << "Legend: "
            << color_add.colorize ("added")
            << ", "
            << color_done.colorize ("completed")
            << ", "
            << color_delete.colorize ("deleted")
            << optionalBlankLine ()
            << "\n";
      else
        out << "Legend: + added, X completed, - deleted\n";
    }
    else
    {
      out << "No tasks.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-ghistory-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportGHistoryAnnual (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-ghistory-command"))
  {
    std::map <time_t, int> groups;          // Represents any month with data
    std::map <time_t, int> addedGroup;      // Additions by month
    std::map <time_t, int> completedGroup;  // Completions by month
    std::map <time_t, int> deletedGroup;    // Deletions by month

    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      Date entry (task->get ("entry"));

      Date end;
      if (task->has ("end"))
        end = Date (task->get ("end"));

      time_t epoch = entry.startOfYear ().toEpoch ();
      groups[epoch] = 0;

      // Every task has an entry date.
      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      // All deleted tasks have an end date.
      if (task->getStatus () == Task::deleted)
      {
        epoch = end.startOfYear ().toEpoch ();
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }

      // All completed tasks have an end date.
      else if (task->getStatus () == Task::completed)
      {
        epoch = end.startOfYear ().toEpoch ();
        groups[epoch] = 0;

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }

    int widthOfBar = context.getWidth () - 5;   // 5 == strlen ("2008 ")

    // Now build the table.
    Table table;
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Year");
    table.addColumn ("Number Added/Completed/Deleted");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
    }
    else
      table.setTableDashedUnderline ();

    Color color_add    (context.config.get ("color.history.add"));
    Color color_done   (context.config.get ("color.history.done"));
    Color color_delete (context.config.get ("color.history.delete"));

    // Determine the longest line, and the longest "added" line.
    int maxAddedLine = 0;
    int maxRemovedLine = 0;
    foreach (i, groups)
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
      foreach (i, groups)
      {
        row = table.addRow ();

        totalAdded     += addedGroup[i->first];
        totalCompleted += completedGroup[i->first];
        totalDeleted   += deletedGroup[i->first];

        Date dt (i->first);
        int m, d, y;
        dt.toMDY (m, d, y);

        if (y != priorYear)
        {
          table.addCell (row, 0, y);
          priorYear = y;
        }

        unsigned int addedBar     = (widthOfBar *     addedGroup[i->first]) / maxLine;
        unsigned int completedBar = (widthOfBar * completedGroup[i->first]) / maxLine;
        unsigned int deletedBar   = (widthOfBar *   deletedGroup[i->first]) / maxLine;

        std::string bar = "";
        if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        {
          char number[24];
          std::string aBar = "";
          if (addedGroup[i->first])
          {
            sprintf (number, "%d", addedGroup[i->first]);
            aBar = number;
            while (aBar.length () < addedBar)
              aBar = " " + aBar;
          }

          std::string cBar = "";
          if (completedGroup[i->first])
          {
            sprintf (number, "%d", completedGroup[i->first]);
            cBar = number;
            while (cBar.length () < completedBar)
              cBar = " " + cBar;
          }

          std::string dBar = "";
          if (deletedGroup[i->first])
          {
            sprintf (number, "%d", deletedGroup[i->first]);
            dBar = number;
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

        table.addCell (row, 1, bar);
      }
    }

    std::stringstream out;
    if (table.rowCount ())
    {
      out << optionalBlankLine ()
          << table.render ()
          << "\n";

      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        out << "Legend: "
            << color_add.colorize ("added")
            << ", "
            << color_done.colorize ("completed")
            << ", "
            << color_delete.colorize ("deleted")
            << optionalBlankLine ()
            << "\n";
      else
        out << "Legend: + added, X completed, - deleted\n";
    }
    else
    {
      out << "No tasks.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-ghistory-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
