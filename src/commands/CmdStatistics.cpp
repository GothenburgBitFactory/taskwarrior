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

#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <ViewText.h>
#include <Duration.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <CmdStatistics.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStatistics::CmdStatistics ()
{
  _keyword     = "stats";
  _usage       = "task stats";
  _description = "Shows task database statistics.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStatistics::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Go get the file sizes.
  size_t dataSize = 0;

  Directory location (context.config.get ("data.location"));
  File pending (location.data + "/pending.data");
  dataSize += pending.size ();

  File completed (location.data + "/completed.data");
  dataSize += completed.size ();

  File undo (location.data + "/undo.data");
  dataSize += undo.size ();

  std::vector <std::string> undoTxns;
  File::read (undo, undoTxns);
  int undoCount = 0;
  std::vector <std::string>::iterator tx;
  for (tx = undoTxns.begin (); tx != undoTxns.end (); ++tx)
    if (tx->substr (0, 3) == "---")
      ++undoCount;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

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

  std::vector <Task>::iterator it;
  for (it = tasks.begin (); it != tasks.end (); ++it)
  {
    ++totalT;
    if (it->getStatus () == Task::deleted)   ++deletedT;
    if (it->getStatus () == Task::pending)   ++pendingT;
    if (it->getStatus () == Task::completed) ++completedT;
    if (it->getStatus () == Task::recurring) ++recurringT;
    if (it->getStatus () == Task::waiting)   ++waitingT;

    time_t entry = strtol (it->get ("entry").c_str (), NULL, 10);
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (it->getStatus () == Task::completed)
    {
      time_t end = strtol (it->get ("end").c_str (), NULL, 10);
      daysPending += (end - entry) / 86400.0;
    }

    if (it->getStatus () == Task::pending)
      daysPending += (now.toEpoch () - entry) / 86400.0;

    descLength += it->get ("description").length ();

    std::vector <Att> annotations;
    it->getAnnotations (annotations);
    annotationsT += annotations.size ();

    std::vector <std::string> tags;
    it->getTags (tags);
    if (tags.size ()) ++taggedT;

    std::vector <std::string>::iterator t;
    for (t = tags.begin (); t != tags.end (); ++t)
      allTags[*t] = 0;

    std::string project = it->get ("project");
    if (project != "")
      allProjects[project] = 0;
  }

  // Create a table for output.
  ViewText view;
  view.width (context.getWidth ());
  view.intraPadding (2);
  view.add (Column::factory ("string", "Category"));
  view.add (Column::factory ("string", "Data"));

  int row = view.addRow ();
  view.set (row, 0, "Pending");
  view.set (row, 1, pendingT);

  row = view.addRow ();
  view.set (row, 0, "Waiting");
  view.set (row, 1, waitingT);

  row = view.addRow ();
  view.set (row, 0, "Recurring");
  view.set (row, 1, recurringT);

  row = view.addRow ();
  view.set (row, 0, "Completed");
  view.set (row, 1, completedT);

  row = view.addRow ();
  view.set (row, 0, "Deleted");
  view.set (row, 1, deletedT);

  row = view.addRow ();
  view.set (row, 0, "Total");
  view.set (row, 1, totalT);

  row = view.addRow ();
  view.set (row, 0, "Annotations");
  view.set (row, 1, annotationsT);

  row = view.addRow ();
  view.set (row, 0, "Unique tags");
  view.set (row, 1, (int)allTags.size ());

  row = view.addRow ();
  view.set (row, 0, "Projects");
  view.set (row, 1, (int)allProjects.size ());

  row = view.addRow ();
  view.set (row, 0, "Data size");
  view.set (row, 1, formatBytes (dataSize));

  row = view.addRow ();
  view.set (row, 0, "Undo transactions");
  view.set (row, 1, undoCount);

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, "Tasks tagged");

    std::stringstream value;
    value << std::setprecision (3) << (100.0 * taggedT / totalT) << "%";
    view.set (row, 1, value.str ());
  }

  if (tasks.size ())
  {
    Date e (earliest);
    row = view.addRow ();
    view.set (row, 0, "Oldest task");
    view.set (row, 1, e.toString (context.config.get ("dateformat")));

    Date l (latest);
    row = view.addRow ();
    view.set (row, 0, "Newest task");
    view.set (row, 1, l.toString (context.config.get ("dateformat")));

    row = view.addRow ();
    view.set (row, 0, "Task used for");
    view.set (row, 1, Duration (latest - earliest).format ());
  }

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, "Task added every");
    view.set (row, 1, Duration (((latest - earliest) / totalT)).format ());
  }

  if (completedT)
  {
    row = view.addRow ();
    view.set (row, 0, "Task completed every");
    view.set (row, 1, Duration ((latest - earliest) / completedT).format ());
  }

  if (deletedT)
  {
    row = view.addRow ();
    view.set (row, 0, "Task deleted every");
    view.set (row, 1, Duration ((latest - earliest) / deletedT).format ());
  }

  if (pendingT || completedT)
  {
    row = view.addRow ();
    view.set (row, 0, "Average time pending");
    view.set (row, 1, Duration ((int) ((daysPending / (pendingT + completedT)) * 86400)).format ());
  }

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, "Average desc length");
    std::stringstream value;
    value << (int) (descLength / totalT) << " characters";
    view.set (row, 1, value.str ());
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
