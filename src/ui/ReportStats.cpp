////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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

#include <sstream>
#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "log.h"
#include "ReportStats.h"
#include "Context.h"
#include "File.h"
#include "Path.h"
#include "util.h"
#include "text.h"
#include "main.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

extern Context context;

static int refreshDelay;  // Override with ui.stats.refresh

////////////////////////////////////////////////////////////////////////////////
// This thread sleeps and updates the time every second.
static void tick (int* arg)
{
  ReportStats* r = (ReportStats *) arg;
  do
  {
    // If the element is deinitialized the window goes away, but the thread does
    // not.  Protect against an attempted redraw of a dead window.
    if (r->window != NULL &&
        r->dataChanged ())
    {
      r->gatherStats ();
      r->redraw ();
    }

    sleep (refreshDelay);
  }
  while (arg);
}

////////////////////////////////////////////////////////////////////////////////
ReportStats::ReportStats ()
: highlight_shown (false)
{
//  logWrite ("ReportStats::ReportStats");
  refreshDelay = context.config.getInteger ("ui.stats.refresh");
  logWrite ("ReportStats::ReportStats refreshDelay=%d", refreshDelay);
  pthread_create (&ticker, NULL, (void*(*)(void*)) tick, (void*) this);
}

////////////////////////////////////////////////////////////////////////////////
ReportStats::~ReportStats ()
{
  pthread_kill (ticker, SIGQUIT);
}

////////////////////////////////////////////////////////////////////////////////
void ReportStats::initialize ()
{
  Element::initialize ();
  scrollok (window, TRUE);
//  idlok (window, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Replace with Sensor object.
bool ReportStats::dataChanged ()
{
  bool status = false;

  struct stat s;
  std::string location = Path::expand (context.config.get ("data.location"));
  std::string file = location + "/pending.data";
  stat (file.c_str (), &s);
  if (s.st_mtime != stat_pending.st_mtime)
  {
    stat_pending = s;
    status = true;
  }

  file = location + "/completed.data";
  stat (file.c_str (), &s);
  if (s.st_mtime != stat_completed.st_mtime)
  {
    stat_completed = s;
    status = true;
  }

  file = location + "/undo.data";
  stat (file.c_str (), &s);
  if (s.st_mtime != stat_undo.st_mtime)
  {
    stat_undo = s;
    status = true;
  }

  if (highlight_shown)
    status = true;

  return status;
}

////////////////////////////////////////////////////////////////////////////////
// Maintains two sets of data, previous and current.  All newly gathered data
// goes into current.  Next iteration, the current is moved to previous.
//
// This two-stage tracking allows detection of changes from one refresh to the
// next, and therefore the highlighting.
void ReportStats::gatherStats ()
{
  // The new becomes the old.
  previous = current;

  // Data size
  size_t dataSize = stat_pending.st_size +
                    stat_completed.st_size +
                    stat_undo.st_size;

  // Undo transactions
  std::string location = Path::expand (context.config.get ("data.location"));
  std::string file = location + "/undo.data";

  std::vector <std::string> undo;
  File::read (file, undo);
  int undoCount = 0;
  std::vector <std::string>::iterator tx;
  for (tx = undo.begin (); tx != undo.end (); ++tx)
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

    time_t entry = atoi (it->get ("entry").c_str ());
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (it->getStatus () == Task::completed)
    {
      time_t end = atoi (it->get ("end").c_str ());
      daysPending += (end - entry) / 86400.0;
    }

    if (it->getStatus () == Task::pending)
      daysPending += (now - entry) / 86400.0;

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

  current["pending"]     = commify (pendingT);
  current["waiting"]     = commify (waitingT);
  current["recurring"]   = commify (recurringT);
  current["completed"]   = commify (completedT);
  current["deleted"]     = commify (deletedT);
  current["total"]       = commify (totalT);
  current["annotations"] = commify (annotationsT);
  current["tags"]        = commify ((int)allTags.size ());
  current["projects"]    = commify ((int)allProjects.size ());
  current["size"]        = formatBytes (dataSize);
  current["undo"]        = commify (undoCount);

  std::stringstream s;
  if (totalT)
    s << std::setprecision (3) << (100.0 * taggedT / totalT);
  else
    s << 0;

  s << "%";
  current["tagged"] = s.str ();

  if (tasks.size ())
  {
    Date e (earliest);
    Date l (latest);

    current["oldest"] = e.toString (context.config.get ("dateformat"));
    current["newest"] = l.toString (context.config.get ("dateformat"));
    current["since"]  = formatSeconds (latest - earliest);
  }
  else
  {
    current["oldest"] = "-";
    current["newest"] = "-";
    current["since"]  = "-";
  }

  if (totalT)
    current["add_every"] = formatSeconds ((latest - earliest) / totalT);
  else
    current["add_every"] = "-";

  if (completedT)
    current["complete_every"] = formatSeconds ((latest - earliest) / completedT);
  else
    current["complete_every"] = "-";

  if (deletedT)
    current["delete_every"] = formatSeconds ((latest - earliest) / deletedT);
  else
    current["delete_every"] = "-";

  if (pendingT || completedT)
    current["time"] = formatSeconds ((int) ((daysPending / (pendingT + completedT)) * 86400));
  else
    current["time"] = "-";

  if (totalT)
  {
    std::stringstream value;
    value << (int) (descLength / totalT) << " characters";
    current["desc"] = value.str ();
  }
  else
    current["desc"] = "-";
}

////////////////////////////////////////////////////////////////////////////////
bool ReportStats::event (int e)
{
  switch (e)
  {
  case KEY_UP:
    logWrite ("ReportStats::event KEY_UP");
    wscrl (window, -1);
    wrefresh (window);
    break;

  case KEY_DOWN:
    logWrite ("ReportStats::event KEY_DOWN");
    wscrl (window, 1);
    wrefresh (window);
    break;

  case KEY_MOUSE:
    {
      MEVENT m;
      getmouse (&m);
      logWrite ("ReportStats::event KEY_MOUSE [%d,%d] %x (%s)",
                m.x,
                m.y,
                m.bstate,
                ((m.x >= left && m.x < left + width && m.y >= top && m.y < top + height) ? "hit" : "miss"));
    }
    return true;
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void ReportStats::redraw ()
{
  logWrite ("ReportStats::redraw");

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR (20) | A_DIM);
  wbkgdset (window, COLOR_PAIR(20) | A_DIM);

  std::stringstream title;
  title << "Live Statistics (updates every "
        << refreshDelay
        << " seconds)";
  mvwaddstr (window, 0, 0, title.str ().c_str ());

  highlight_shown = false;
  renderItem (2,  24, "Pending tasks",        "pending");
  renderItem (3,  24, "Waiting tasks",        "waiting");
  renderItem (4,  24, "Recurring tasks",      "recurring");
  renderItem (5,  24, "Completed tasks",      "completed");
  renderItem (6,  24, "Deleted tasks",        "deleted");
  renderItem (7,  24, "Total tasks",          "total");
  renderItem (8,  24, "Total annotations",    "annotations");
  renderItem (9,  24, "Unique tags",          "tags");
  renderItem (10, 24, "Projects",             "projects");
  renderItem (11, 24, "Data size",            "size");
  renderItem (12, 24, "Undo transactions",    "undo");
  renderItem (13, 24, "Tasks tagged",         "tagged");
  renderItem (14, 24, "Oldest task",          "oldest");
  renderItem (15, 24, "Newest task",          "newest");
  renderItem (16, 24, "Task used for",        "since");
  renderItem (17, 24, "Task added every",     "add_every");
  renderItem (18, 24, "Task completed every", "complete_every");
  renderItem (19, 24, "Task deleted every",   "delete_every");
  renderItem (20, 24, "Average time pending", "time");
  renderItem (21, 24, "Average desc length",  "desc");

  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
void ReportStats::renderItem (
  int row,
  int col,
  const std::string& description,
  const std::string& key)
{
  wbkgdset (window, COLOR_PAIR(20) | A_DIM);
  mvwaddstr (window, row, 0, description.c_str ());

  if (current[key] != previous[key])
  {
    wbkgdset (window, COLOR_PAIR(3) | A_BOLD);
    highlight_shown = true;
  }

  mvwaddstr (window, row, col, current[key].c_str ());
}

////////////////////////////////////////////////////////////////////////////////
