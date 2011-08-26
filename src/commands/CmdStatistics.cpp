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
#include <iomanip>
#include <stdlib.h>
#include <ViewText.h>
#include <Duration.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <CmdStatistics.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStatistics::CmdStatistics ()
{
  _keyword     = "stats";
  _usage       = "task stats [<filter>]";
  _description = STRING_CMD_STATS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStatistics::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  std::string dateformat = context.config.get ("dateformat");

  // Go get the file sizes.
  size_t dataSize = 0;

  Directory location (context.config.get ("data.location"));
  File pending (location._data + "/pending.data");
  dataSize += pending.size ();

  File completed (location._data + "/completed.data");
  dataSize += completed.size ();

  File undo (location._data + "/undo.data");
  dataSize += undo.size ();

  std::vector <std::string> undoTxns;
  File::read (undo, undoTxns);
  int undoCount = 0;
  std::vector <std::string>::iterator tx;
  for (tx = undoTxns.begin (); tx != undoTxns.end (); ++tx)
    if (tx->substr (0, 3) == "---")
      ++undoCount;

  // Get all the tasks.
  std::vector <Task> filtered;
  filter (context.tdb2.pending.get_tasks (), filtered);
  filter (context.tdb2.completed.get_tasks (), filtered);

  Date now;
  time_t earliest   = time (NULL);
  time_t latest     = 1;
  int totalT        = 0;
  int deletedT      = 0;
  int pendingT      = 0;
  int completedT    = 0;
  int waitingT      = 0;
  int taggedT       = 0;
  int annotationsT  = 0;
  int recurringT    = 0;
  float daysPending = 0.0;
  int descLength    = 0;
  std::map <std::string, int> allTags;
  std::map <std::string, int> allProjects;

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    ++totalT;
    if (task->getStatus () == Task::deleted)   ++deletedT;
    if (task->getStatus () == Task::pending)   ++pendingT;
    if (task->getStatus () == Task::completed) ++completedT;
    if (task->getStatus () == Task::recurring) ++recurringT;
    if (task->getStatus () == Task::waiting)   ++waitingT;

    time_t entry = strtol (task->get ("entry").c_str (), NULL, 10);
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (task->getStatus () == Task::completed)
    {
      time_t end = strtol (task->get ("end").c_str (), NULL, 10);
      daysPending += (end - entry) / 86400.0;
    }

    if (task->getStatus () == Task::pending)
      daysPending += (now.toEpoch () - entry) / 86400.0;

    descLength += task->get ("description").length ();

    std::map <std::string, std::string> annotations;
    task->getAnnotations (annotations);
    annotationsT += annotations.size ();

    std::vector <std::string> tags;
    task->getTags (tags);
    if (tags.size ()) ++taggedT;

    std::vector <std::string>::iterator t;
    for (t = tags.begin (); t != tags.end (); ++t)
      allTags[*t] = 0;

    std::string project = task->get ("project");
    if (project != "")
      allProjects[project] = 0;
  }

  // Create a table for output.
  ViewText view;
  view.width (context.getWidth ());
  view.intraPadding (2);
  view.add (Column::factory ("string", STRING_CMD_STATS_CATEGORY));
  view.add (Column::factory ("string", STRING_CMD_STATS_DATA));

  int row = view.addRow ();
  view.set (row, 0, STRING_COLUMN_LABEL_STAT_PE);
  view.set (row, 1, pendingT);

  row = view.addRow ();
  view.set (row, 0, STRING_COLUMN_LABEL_STAT_WA);
  view.set (row, 1, waitingT);

  row = view.addRow ();
  view.set (row, 0, STRING_COLUMN_LABEL_STAT_RE);
  view.set (row, 1, recurringT);

  row = view.addRow ();
  view.set (row, 0, STRING_COLUMN_LABEL_STAT_CO);
  view.set (row, 1, completedT);

  row = view.addRow ();
  view.set (row, 0, STRING_COLUMN_LABEL_STAT_DE);
  view.set (row, 1, deletedT);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_TOTAL);
  view.set (row, 1, totalT);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_ANNOTATIONS);
  view.set (row, 1, annotationsT);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_UNIQUE_TAGS);
  view.set (row, 1, (int)allTags.size ());

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_PROJECTS);
  view.set (row, 1, (int)allProjects.size ());

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_DATA_SIZE);
  view.set (row, 1, formatBytes (dataSize));

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_UNDO_TXNS);
  view.set (row, 1, undoCount);

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_TAGGED);

    std::stringstream value;
    value << std::setprecision (3) << (100.0 * taggedT / totalT) << "%";
    view.set (row, 1, value.str ());
  }

  if (filtered.size ())
  {
    Date e (earliest);
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_OLDEST);
    view.set (row, 1, e.toString (dateformat));

    Date l (latest);
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_NEWEST);
    view.set (row, 1, l.toString (dateformat));

    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_USED_FOR);
    view.set (row, 1, Duration (latest - earliest).format ());
  }

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_ADD_EVERY);
    view.set (row, 1, Duration (((latest - earliest) / totalT)).format ());
  }

  if (completedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_COMP_EVERY);
    view.set (row, 1, Duration ((latest - earliest) / completedT).format ());
  }

  if (deletedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_DEL_EVERY);
    view.set (row, 1, Duration ((latest - earliest) / deletedT).format ());
  }

  if (pendingT || completedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_AVG_PEND);
    view.set (row, 1, Duration ((int) ((daysPending / (pendingT + completedT)) * 86400)).format ());
  }

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_DESC_LEN);
    view.set (row, 1, format (STRING_CMD_STATS_CHARS, (int) (descLength / totalT)));
  }

  // If an alternating row color is specified, notify the table.
  if (context.color ())
  {
    Color alternate (context.config.get ("color.alternate"));
    if (alternate.nontrivial ())
    {
      view.colorOdd (alternate);
      view.intraColorOdd (alternate);
      view.extraColorOdd (alternate);
    }
  }

  out << optionalBlankLine ()
      << view.render ()
      << optionalBlankLine ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
