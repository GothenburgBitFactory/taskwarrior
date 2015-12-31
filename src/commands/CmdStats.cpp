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
#include <CmdStats.h>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <ViewText.h>
#include <ISO8601.h>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStats::CmdStats ()
{
  _keyword               = "stats";
  _usage                 = "task <filter> stats";
  _description           = STRING_CMD_STATS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStats::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  std::string dateformat = context.config.get ("dateformat");

  // Go get the file sizes.
  size_t dataSize = context.tdb2.pending._file.size ()
                  + context.tdb2.completed._file.size ()
                  + context.tdb2.undo._file.size ()
                  + context.tdb2.backlog._file.size ();

  // Count the undo transactions.
  std::vector <std::string> undoTxns = context.tdb2.undo.get_lines ();
  int undoCount = 0;
  for (auto& tx : undoTxns)
    if (tx == "---")
      ++undoCount;

  // Count the backlog transactions.
  std::vector <std::string> backlogTxns = context.tdb2.backlog.get_lines ();
  int backlogCount = 0;
  for (auto& tx : backlogTxns)
    if (tx[0] == '{')
      ++backlogCount;

  // Get all the tasks.
  Filter filter;
  std::vector <Task> all = context.tdb2.all_tasks ();
  std::vector <Task> filtered;
  filter.subset (all, filtered);

  ISO8601d now;
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
  int blockingT     = 0;
  int blockedT      = 0;
  float daysPending = 0.0;
  int descLength    = 0;
  std::map <std::string, int> allTags;
  std::map <std::string, int> allProjects;

  for (auto& task : filtered)
  {
    ++totalT;

    Task::status status = task.getStatus ();
    switch (status)
    {
    case Task::deleted:   ++deletedT;   break;
    case Task::pending:   ++pendingT;   break;
    case Task::completed: ++completedT; break;
    case Task::recurring: ++recurringT; break;
    case Task::waiting:   ++waitingT;   break;
    }

    if (task.is_blocked)  ++blockedT;
    if (task.is_blocking) ++blockingT;

    time_t entry = strtol (task.get ("entry").c_str (), NULL, 10);
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (status == Task::completed)
    {
      time_t end = strtol (task.get ("end").c_str (), NULL, 10);
      daysPending += (end - entry) / 86400.0;
    }

    if (status == Task::pending)
      daysPending += (now.toEpoch () - entry) / 86400.0;

    descLength += task.get ("description").length ();

    std::map <std::string, std::string> annotations;
    task.getAnnotations (annotations);
    annotationsT += annotations.size ();

    std::vector <std::string> tags;
    task.getTags (tags);
    if (tags.size ())
      ++taggedT;

    for (auto& tag : tags)
      allTags[tag] = 0;

    std::string project = task.get ("project");
    if (project != "")
      allProjects[project] = 0;
  }

  // Create a table for output.
  ViewText view;
  view.width (context.getWidth ());
  view.intraPadding (2);
  view.add (Column::factory ("string", STRING_CMD_STATS_CATEGORY));
  view.add (Column::factory ("string", STRING_CMD_STATS_DATA));

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

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
  view.set (row, 0, STRING_CMD_STATS_BLOCKED);
  view.set (row, 1, blockedT);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_BLOCKING);
  view.set (row, 1, blockingT);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_DATA_SIZE);
  view.set (row, 1, formatBytes (dataSize));

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_UNDO_TXNS);
  view.set (row, 1, undoCount);

  row = view.addRow ();
  view.set (row, 0, STRING_CMD_STATS_BACKLOG);
  view.set (row, 1, backlogCount);

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
    ISO8601d e (earliest);
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_OLDEST);
    view.set (row, 1, e.toString (dateformat));

    ISO8601d l (latest);
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_NEWEST);
    view.set (row, 1, l.toString (dateformat));

    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_USED_FOR);
    view.set (row, 1, ISO8601p (latest - earliest).formatVague ());
  }

  if (totalT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_ADD_EVERY);
    view.set (row, 1, ISO8601p (((latest - earliest) / totalT)).formatVague ());
  }

  if (completedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_COMP_EVERY);
    view.set (row, 1, ISO8601p ((latest - earliest) / completedT).formatVague ());
  }

  if (deletedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_DEL_EVERY);
    view.set (row, 1, ISO8601p ((latest - earliest) / deletedT).formatVague ());
  }

  if (pendingT || completedT)
  {
    row = view.addRow ();
    view.set (row, 0, STRING_CMD_STATS_AVG_PEND);
    view.set (row, 1, ISO8601p ((int) ((daysPending / (pendingT + completedT)) * 86400)).formatVague ());
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
