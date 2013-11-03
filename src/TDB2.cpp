////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <algorithm>
#include <list>
#include <set>
#include <stdlib.h>
#include <signal.h>
#include <Context.h>
#include <Color.h>
#include <Date.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <TDB2.h>

extern Context context;

#define NDEBUG
#include <assert.h>
#include <Taskmod.h>

#define DEBUG_OUTPUT 0

#if DEBUG_OUTPUT > 0
  #define DEBUG_STR(str)       std::cerr << "DEBUG: " << str << "\n"; std::cerr.flush()
  #define DEBUG_STR_PART(str)  std::cerr << "DEBUG: " << str; std::cerr.flush()
  #define DEBUG_STR_END(str)   std::cerr << str << "\n"; std::cerr.flush()
#else
  #define DEBUG_STR(str)
  #define DEBUG_STR_PART(str)
  #define DEBUG_STR_END(str)
#endif

////////////////////////////////////////////////////////////////////////////////
TF2::TF2 ()
: _read_only (false)
, _dirty (false)
, _loaded_tasks (false)
, _loaded_lines (false)
, _has_ids (false)
, _auto_dep_scan (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TF2::~TF2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
void TF2::target (const std::string& f)
{
  _file = File (f);

  // A missing file is not considered unwritable.
  _read_only = false;
  if (_file.exists () && ! _file.writable ())
    _read_only = true;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TF2::get_tasks ()
{
  if (! _loaded_tasks)
    load_tasks ();

  return _tasks;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string>& TF2::get_lines ()
{
  if (! _loaded_lines)
    load_lines ();

  return _lines;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by id.
bool TF2::get (int id, Task& task)
{
  if (! _loaded_tasks)
    load_tasks ();

  // This is an optimization.  Since the 'id' is based on the line number of
  // pending.data file, the task in question cannot appear earlier than line
  // (id - 1) in the file.  It can, however, appear significantly later because
  // it is not known how recent a GC operation was run.
  for (int i = id - 1; i < _tasks.size (); ++i)
  {
    if (_tasks[i].id == id)
    {
      task = _tasks[i];
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by uuid.
bool TF2::get (const std::string& uuid, Task& task)
{
  if (! _loaded_tasks)
    load_tasks ();

  std::vector <Task>::iterator i;
  for (i = _tasks.begin (); i != _tasks.end (); ++i)
  {
    if (i->get ("uuid") == uuid)
    {
      task = *i;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_task (const Task& task)
{
  _tasks.push_back (task);           // For subsequent queries
  _added_tasks.push_back (task);     // For commit/synch

/* TODO handle 'add' and 'log'.
  int id = context.tdb2.next_id ();
  _I2U[id] = task.get ("uuid");
  _U2I[task.get ("uuid")] = id;
*/

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
bool TF2::modify_task (const Task& task)
{
  // Modify in-place.
  std::string uuid = task.get ("uuid");
  std::vector <Task>::iterator i;
  for (i = _tasks.begin (); i != _tasks.end (); ++i)
  {
    if (i->get ("uuid") == uuid)
    {
      *i = task;
      _modified_tasks.push_back (task);
      _dirty = true;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_line (const std::string& line)
{
  _lines.push_back (line);
  _added_lines.push_back (line);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::clear_tasks ()
{
  _tasks.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::clear_lines ()
{
  _lines.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Top-down recomposition.
void TF2::commit ()
{
  // The _dirty flag indicates that the file needs to be written.
  if (_dirty)
  {
    // Special case: added but no modified means just append to the file.
    if (!_modified_tasks.size () &&
        (_added_tasks.size () || _added_lines.size ()))
    {
      if (_file.open ())
      {
        if (context.config.getBoolean ("locking"))
          _file.waitForLock ();

        // Write out all the added tasks.
        std::vector <Task>::iterator task;
        for (task = _added_tasks.begin ();
             task != _added_tasks.end ();
             ++task)
        {
          _file.append (task->composeF4 () + "\n");
        }

        _added_tasks.clear ();

        // Write out all the added lines.
        std::vector <std::string>::iterator line;
        for (line = _added_lines.begin ();
             line != _added_lines.end ();
             ++line)
        {
          _file.append (*line);
        }

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
    else
    {
      if (_file.open ())
      {
        // Truncate the file and rewrite.
        _file.truncate ();

        // Only write out _tasks, because any deltas have already been applied.
        std::vector <Task>::iterator task;
        for (task = _tasks.begin ();
             task != _tasks.end ();
             ++task)
        {
          _file.append (task->composeF4 () + "\n");
        }

        // Write out all the added lines.
        std::vector <std::string>::iterator line;
        for (line = _added_lines.begin ();
             line != _added_lines.end ();
             ++line)
        {
          _file.append (*line);
        }

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks ()
{
  context.timer_load.start ();

  if (! _loaded_lines)
  {
    load_lines ();

    // Apply previously added lines.
    std::vector <std::string>::iterator i;
    for (i = _added_lines.begin (); i != _added_lines.end (); ++i)
      _lines.push_back (*i);
  }

  int line_number = 0;
  try
  {
    // Reduce unnecessary allocations/copies.
    _tasks.reserve (_lines.size ());

    std::vector <std::string>::iterator i;
    for (i = _lines.begin (); i != _lines.end (); ++i)
    {
      ++line_number;
      Task task (*i);

      // Some tasks gets an ID.
      if (_has_ids)
        task.id = context.tdb2.next_id ();

      _tasks.push_back (task);

      // Maintain mapping for ease of link/dependency resolution.
      // Note that this mapping is not restricted by the filter, and is
      // therefore a complete set.
      if (task.id)
      {
        _I2U[task.id] = task.get ("uuid");
        _U2I[task.get ("uuid")] = task.id;
      }
    }

    if (_auto_dep_scan)
      dependency_scan ();

    _loaded_tasks = true;
  }

  catch (const std::string& e)
  {
    throw e + format (STRING_TDB2_PARSE_ERROR, _file._data, line_number);
  }

  context.timer_load.stop ();
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines ()
{
  if (_file.open ())
  {
    if (context.config.getBoolean ("locking"))
      _file.waitForLock ();

    _file.read (_lines);
    _file.close ();
    _loaded_lines = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string TF2::uuid (int id)
{
  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    std::vector <Task>::iterator i;
    for (i = _added_tasks.begin (); i != _added_tasks.end (); ++i)
      _tasks.push_back (*i);
  }

  std::map <int, std::string>::const_iterator i;
  if ((i = _I2U.find (id)) != _I2U.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int TF2::id (const std::string& uuid)
{
  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    std::vector <Task>::iterator i;
    for (i = _added_tasks.begin (); i != _added_tasks.end (); ++i)
      _tasks.push_back (*i);
  }

  std::map <std::string, int>::const_iterator i;
  if ((i = _U2I.find (uuid)) != _U2I.end ())
    return i->second;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::has_ids ()
{
  _has_ids = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::auto_dep_scan ()
{
  _auto_dep_scan = true;
}

////////////////////////////////////////////////////////////////////////////////
// Completely wipe it all clean.
void TF2::clear ()
{
  _read_only       = false;
  _dirty           = false;
  _loaded_tasks    = false;
  _loaded_lines    = false;

  // Note that these are deliberately not cleared.
  //_file._data      = "";
  //_has_ids         = false;
  //_auto_dep_scan   = false;

  _tasks.clear ();
  _added_tasks.clear ();
  _modified_tasks.clear ();
  _lines.clear ();
  _added_lines.clear ();
  _I2U.clear ();
  _U2I.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// For any task that has depenencies, follow the chain of dependencies until the
// end.  Along the way, update the Task::is_blocked and Task::is_blocking data
// cache.
void TF2::dependency_scan ()
{
  // Iterate and modify TDB2 in-place.  Don't do this at home.
  std::vector <Task>::iterator left;
  for (left  = _tasks.begin ();
       left != _tasks.end ();
       ++left)
  {
    if (left->has ("depends"))
    {
      std::vector <std::string> deps;
      left->getDependencies (deps);

      std::vector <std::string>::iterator d;
      for (d = deps.begin (); d != deps.end (); ++d)
      {
        std::vector <Task>::iterator right;
        for (right  = _tasks.begin ();
             right != _tasks.end ();
             ++right)
        {
          if (right->get ("uuid") == *d)
          {
            Task::status status = right->getStatus ();
            if (status != Task::completed &&
                status != Task::deleted)
            {
              left->is_blocked = true;
              right->is_blocking = true;
            }

            break;
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::string TF2::dump ()
{
  Color red    ("rgb500 on rgb100");
  Color yellow ("rgb550 on rgb220");
  Color green  ("rgb050 on rgb010");

  // File label.
  std::string label;
  std::string::size_type slash = _file._data.rfind ('/');
  if (slash != std::string::npos)
    label = rightJustify (_file._data.substr (slash + 1), 14);

  // File mode.
  std::string mode = std::string (_file.readable () ? "r" : "-") +
                     std::string (_file.writable () ? "w" : "-");
       if (mode == "r-") mode = red.colorize (mode);
  else if (mode == "rw") mode = green.colorize (mode);
  else                   mode = yellow.colorize (mode);

  // Hygiene.
  std::string hygiene = _dirty ? red.colorize ("O") : green.colorize ("-");

  std::string tasks          = green.colorize  (rightJustifyZero ((int) _tasks.size (),          4));
  std::string tasks_added    = red.colorize    (rightJustifyZero ((int) _added_tasks.size (),    3));
  std::string tasks_modified = yellow.colorize (rightJustifyZero ((int) _modified_tasks.size (), 3));
  std::string lines          = green.colorize  (rightJustifyZero ((int) _lines.size (),          4));
  std::string lines_added    = red.colorize    (rightJustifyZero ((int) _added_lines.size (),    3));

  char buffer[256];  // Composed string is actually 246 bytes.  Yikes.
  snprintf (buffer, 256, "%14s %s %s T%s+%s~%s L%s+%s",
            label.c_str (),
            mode.c_str (),
            hygiene.c_str (),
            tasks.c_str (),
            tasks_added.c_str (),
            tasks_modified.c_str (),
            lines.c_str (),
            lines_added.c_str ());

  return std::string (buffer);
}

////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 ()
: _location ("")
, _id (1)
{
  // Mark the pending file as the only one that has ID numbers.
  pending.has_ids ();

  // Indicate that dependencies should be automatically scanned on startup,
  // setting Task::is_blocked and Task::is_blocking accordingly.
  pending.auto_dep_scan ();
}

////////////////////////////////////////////////////////////////////////////////
// Deliberately no file writes on destruct.  Commit should have been already
// called, if data is to be preserved.
TDB2::~TDB2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Once a location is known, the files can be set up.  Note that they are not
// read.
void TDB2::set_location (const std::string& location)
{
  _location = location;

  pending.target   (location + "/pending.data");
  completed.target (location + "/completed.data");
  undo.target      (location + "/undo.data");
  backlog.target   (location + "/backlog.data");
}

////////////////////////////////////////////////////////////////////////////////
// Add the new task to the appropriate file.
void TDB2::add (Task& task, bool add_to_backlog /* = true */)
{
  // Ensure the task is consistent, and provide defaults if necessary.
  task.validate ();

  // If the tasks are loaded, then verify that this uuid is not already in
  // the file.
  if (!verifyUniqueUUID (task.get ("uuid")))
    throw format (STRING_TDB2_UUID_NOT_UNIQUE, task.get ("uuid"));

  // Add new task to either pending or completed.
  std::string status = task.get ("status");
  if (status == "completed" ||
      status == "deleted")
    completed.add_task (task);
  else
    pending.add_task (task);

  // Add undo data lines:
  //   time <time>
  //   new <task>
  //   ---
  undo.add_line ("time " + Date ().toEpochString () + "\n");
  undo.add_line ("new " + task.composeF4 () + "\n");
  undo.add_line ("---\n");

  // Add task to backlog.
  if (add_to_backlog)
    backlog.add_line (task.composeJSON () + "\n");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::modify (Task& task, bool add_to_backlog /* = true */)
{
  // Ensure the task is consistent, and provide defaults if necessary.
  task.validate (false);

  // All modified tasks are timestamped.
  task.setModified ();

  // Find task, overwrite it.
  Task original;
  get (task.get ("uuid"), original);

  if (taskDiff (original, task))
  {
    // Update the task, wherever it is.
    if (!pending.modify_task (task))
      completed.modify_task (task);

    // time <time>
    // old <task>
    // new <task>
    // ---
    undo.add_line ("time " + Date ().toEpochString () + "\n");
    undo.add_line ("old " + original.composeF4 () + "\n");
    undo.add_line ("new " + task.composeF4 () + "\n");
    undo.add_line ("---\n");

    // Add modified task to backlog.
    if (add_to_backlog)
      backlog.add_line (task.composeJSON () + "\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::commit ()
{
  // Ignore harmful signals.
  signal (SIGHUP,    SIG_IGN);
  signal (SIGINT,    SIG_IGN);
  signal (SIGKILL,   SIG_IGN);
  signal (SIGPIPE,   SIG_IGN);
  signal (SIGTERM,   SIG_IGN);
  signal (SIGUSR1,   SIG_IGN);
  signal (SIGUSR2,   SIG_IGN);

  dump ();
  context.timer_commit.start ();

  pending.commit ();
  completed.commit ();
  undo.commit ();
  backlog.commit ();

  // Restore signal handling.
  signal (SIGHUP,    SIG_DFL);
  signal (SIGINT,    SIG_DFL);
  signal (SIGKILL,   SIG_DFL);
  signal (SIGPIPE,   SIG_DFL);
  signal (SIGTERM,   SIG_DFL);
  signal (SIGUSR1,   SIG_DFL);
  signal (SIGUSR2,   SIG_DFL);

  context.timer_commit.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Helper function for TDB::merge
void readTaskmods (std::vector <std::string> &input,
                   std::vector <std::string>::iterator &start,
                   std::list<Taskmod> &list)
{
  static int   resourceID = 1;
  std::string  line;
  Taskmod      tmod_tmp(resourceID++);

  DEBUG_STR ("reading taskmods from file: ");

  for ( ; start != input.end (); ++start)
  {
    line = *start;

    if (line.substr (0, 4) == "time")
    {
      std::stringstream stream (line.substr (5));
      long ts;
      stream >> ts;

      if (stream.fail ())
        throw std::string (STRING_TDB2_UNDO_TIMESTAMP);

      // 'time' is the first line of a modification
      // thus we will (re)set the taskmod object
      tmod_tmp.reset (ts);

    }
    else if (line.substr (0, 3) == "old")
    {
      tmod_tmp.setBefore (Task (line.substr (4)));

    }
    else if (line.substr (0, 3) == "new")
    {
      tmod_tmp.setAfter (Task (line.substr (4)));

      // 'new' is the last line of a modification,
      // thus we can push to the list
      list.push_back (tmod_tmp);

      assert (tmod_tmp.isValid ());
      DEBUG_STR ("  taskmod complete");
    }
  }

  DEBUG_STR ("DONE");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::merge (const std::string& mergeFile)
{
  ///////////////////////////////////////
  // Copyright 2010 - 2013, Johannes Schlatow.
  ///////////////////////////////////////

  // list of modifications that we want to add to the local database
  std::list<Taskmod> mods;

  // list of modifications that we want to add to the local history
  std::list<Taskmod> mods_history;

  // list of modifications on the local database
  // has to be merged with mods to create the new undo.data
  std::list<Taskmod> lmods;

  // will contain the NEW undo.data
  std::vector <std::string> undo_lines;

  ///////////////////////////////////////
  // initialize the files:

  // load merge file (undo file of right/remote branch)
  std::vector <std::string> r;
  if (! File::read (mergeFile, r))
    throw format (STRING_TDB2_UNREADABLE, mergeFile);

  // file has to contain at least one entry
  if (r.size () < 3)
    throw std::string (STRING_TDB2_NO_CHANGES);

  if (! undo._file.exists ())
    undo._file.create ();

  // load undo file (left/local branch)
  std::vector <std::string> l;
  if (! File::read (undo._file._data, l))
    throw format (STRING_TDB2_UNREADABLE, undo._file._data);

  std::string rline, lline;
  std::vector <std::string>::iterator rit, lit;

  // read first line
  rit = r.begin ();
  lit = l.begin ();

  if (rit != r.end())
    rline = *rit;
  if (lit != l.end())
    lline = *lit;

  ///////////////////////////////////////
  // find the branch-off point:

  // first mods are not equal => assuming mergeFile starts at a
  // later point in time
  if (lline.compare (rline) == 0) {
    std::vector<std::string>::const_iterator tmp_lit = lit;
    std::vector<std::string>::const_iterator tmp_rit = rit;
    tmp_lit++;
    tmp_rit++;

    int lookahead = 1;
    if (tmp_lit->substr (0, 3) == "old") {
      lookahead = 2;
    }

    while (lookahead--) {
      if (tmp_lit->compare(*tmp_rit) != 0) {
        break;
      }
      tmp_lit++;
      tmp_rit++;
    }

    if (lookahead == -1) {
      // at this point we know that the first lines are the same
      undo_lines.push_back (lline + "\n");
    }
  }

  // Add some color.
  Color colorAdded    (context.config.get ("color.sync.added"));
  Color colorChanged  (context.config.get ("color.sync.changed"));
  Color colorRejected (context.config.get ("color.sync.rejected"));

  // at this point we can assume: (lline==rline) || (lit == l.end())
  // thus we search for the first non-equal lines or the EOF
  bool found = false;
  for (std::vector<std::string>::const_iterator tmp_lit = lit, tmp_rit = rit;
       (tmp_lit != l.end ()) && (tmp_rit != r.end ());
       ++tmp_lit, ++tmp_rit)
  {
    lline = *tmp_lit;
    rline = *tmp_rit;

    // found first non-matching lines?
    if (lline.compare (rline) != 0)
    {
      found = true;
      break;
    }
    else if (tmp_lit->substr (0, 3) == "---")
    {
      while (lit != tmp_lit)
      {
        ++lit;
        ++rit;
        undo_lines.push_back(*lit + "\n");
      }
      // at this point, all iterators (tmp_{lit,rit}, lit and rit) are at the same line
    }
  }

  if (!found)
  {
    // set iterators to r.end() or l.end() if they point to the last line
    if (++rit != r.end())
      --rit;

    if (++lit != l.end())
      --lit;
  }

  ///////////////////////////////////////
  // branch-off point found:
  if (found)
  {
    DEBUG_STR_PART ("Branch-off point found at: ");
    DEBUG_STR_END (lline);

    std::list<Taskmod> rmods;

    // helper lists
    std::set<std::string> uuid_new, uuid_left;

    // 1. read taskmods out of the remaining lines
    readTaskmods (r, rit, rmods);
    readTaskmods (l, lit, lmods);

    // 2. move new uuids into mods
    DEBUG_STR_PART ("adding new uuids (left) to skip list...");

    // modifications on the left side are already in the database
    // we just need them to merge conflicts, so we add the mods for
    // new uuids to the skip-list 'uuid_left'
    std::list<Taskmod>::iterator lmod_it;
    for (lmod_it = lmods.begin (); lmod_it != lmods.end (); lmod_it++)
    {
      if (lmod_it->isNew ())
      {
/*
        // TODO Don't forget L10N.
        std::cout << "New local task                     "
                  << (context.color () ? colorAdded.colorize (lmod_it->getUuid ()) : lmod_it->getUuid ())
                  << "\n";
*/

        uuid_left.insert (lmod_it->getUuid ());
      }
    }

    DEBUG_STR_END ("done");
    DEBUG_STR_PART ("move new uuids (right) to redo list...");

    // new items on the right side need to be inserted into the
    // local database
    std::list<Taskmod>::iterator rmod_it;
    for (rmod_it = rmods.begin (); rmod_it != rmods.end (); )
    {
      // we have to save and increment the iterator because we may want to delete
      // the object from the list
      std::list<Taskmod>::iterator current = rmod_it++;
      Taskmod tmod = *current;

      if (uuid_left.find (tmod.getUuid ()) != uuid_left.end ())
      {
        // check whether the remote side has added a task with the same UUID
        //  this happens if it inserted a modification with an older timestamp
        //  into the undo.data and thereby moved the branch point to an earlier
        //  point in time. Normally this case will be solved by the merge logic,
        //  BUT if the UUID is considered new the merge logic will be skipped.
        //
        //  This flaw resulted in a couple of duplication issues and bloated
        //  undo files (e.g. #1104).
        //
        //  This is just a "hack" which discards all the modifications of the
        //  remote side to UUIDs that are considered new by both sides.
        //  There may be more issues with the algorithm; probably a redesign
        //  and proper encapsulation of the merge algorithm is due.

        rmods.erase(current);
      }
      else if (tmod.isNew ())
      {
        // new uuid?
/*
        // TODO Don't forget L10N.
        std::cout << "Adding new remote task             "
                  << (context.color () ? colorAdded.colorize (tmod.getUuid ()) : tmod.getUuid ())
                  << "\n";
*/

        uuid_new.insert (tmod.getUuid ());
        mods.push_back (tmod);
        rmods.erase (current);
      }
      else if (uuid_new.find (tmod.getUuid ()) != uuid_new.end ())
      {
        // uuid of modification was new
        mods.push_back (tmod);
        rmods.erase (current);
      }
    }

    DEBUG_STR_END ("done");

    ///////////////////////////////////////
    // merge modifications:
    DEBUG_STR ("Merging modifications:");

    // we iterate backwards to resolve conflicts by timestamps (newest one wins)
    std::list<Taskmod>::reverse_iterator lmod_rit;
    std::list<Taskmod>::reverse_iterator rmod_rit;
    for (lmod_rit = lmods.rbegin (); lmod_rit != lmods.rend (); ++lmod_rit)
    {
      Taskmod     tmod_l = *lmod_rit;
      std::string uuid   = tmod_l.getUuid ();

      DEBUG_STR ("  left uuid: " + uuid);

      // skip if uuid had already been merged
      if (uuid_left.find (uuid) == uuid_left.end ())
      {
        bool rwin = false;
        bool lwin = false;
        for (rmod_rit = rmods.rbegin (); rmod_rit != rmods.rend (); rmod_rit++)
        {
          Taskmod tmod_r = *rmod_rit;

          DEBUG_STR ("    right uuid: " + tmod_r.getUuid ());
          if (tmod_r.getUuid () == uuid)
          {
            DEBUG_STR ("    uuid match found for " + uuid);

            // we already decided to take the mods from the right side
            // but we have to find the first modification newer than
            // the one on the left side to merge the history too
            if (rwin)
            {
              DEBUG_STR ("    scanning right side");
              if (tmod_r > tmod_l)
                mods.push_front (tmod_r);

              std::list<Taskmod>::iterator tmp_it = rmod_rit.base ();
              rmods.erase (--tmp_it);
              rmod_rit--;
            }
            else if (lwin)
            {
              DEBUG_STR ("    cleaning up right side");

              // add tmod_r to local history
              mods_history.push_front (tmod_r);

              std::list<Taskmod>::iterator tmp_it = rmod_rit.base ();
              rmods.erase (--tmp_it);
              rmod_rit--;
            }
            else
            {
              // which one is newer?
              if (tmod_r > tmod_l)
              {
                std::cout << format (STRING_TDB2_REMOTE_CHANGE,
                                     (context.color () ? colorChanged.colorize (uuid) : uuid),
                                     cutOff (tmod_r.getBefore ().get ("description"), 10))
                          << "\n";

                mods.push_front(tmod_r);

                // delete tmod from right side
                std::list<Taskmod>::iterator tmp_it = rmod_rit.base ();
                rmods.erase (--tmp_it);
                rmod_rit--;

                rwin = true;
              }
              else
              {
                std::cout << format (STRING_TDB2_LOCAL_CHANGE,
                                     (context.color () ? colorChanged.colorize (uuid) : uuid),
                                     cutOff (tmod_l.getBefore ().get ("description"), 10))
                          << "\n";

                // inserting right mod into history of local database
                // so that it can be restored later
                // AND more important: create a history that looks the same
                // as if we switched the roles 'remote' and 'local'

                // thus we have to find the oldest change on the local branch that is not on remote branch
                std::list<Taskmod>::iterator lmod_it;
                std::list<Taskmod>::iterator last = lmod_it;
                for (lmod_it = lmods.begin (); lmod_it != lmods.end (); ++lmod_it) {
                  if ((*lmod_it).getUuid () == uuid) {
                    last = lmod_it;
                  }
                }

                if (tmod_l > tmod_r) { // local change is newer
                  last->setBefore(tmod_r.getAfter ());

                  // add tmod_r to local history
                  lmods.push_back(tmod_r);
                }
                else { // both mods have equal timestamps
                  // in this case the local branch wins as above, but the remote change with the
                  // same timestamp will be discarded

                  // find next (i.e. older) mod of this uuid on remote side
                  std::list<Taskmod>::reverse_iterator rmod_rit2;
                  for (rmod_rit2 = rmod_rit, ++rmod_rit2; rmod_rit2 != rmods.rend (); ++rmod_rit2) {
                    Taskmod tmp_mod = *rmod_rit2;
                    if (tmp_mod.getUuid () == uuid) {
                      last->setBefore (tmp_mod.getAfter ());
                      break;
                    }
                  }
                }

                // TODO feature: restore command? We would have to add a marker to the undo.file.

                // delete tmod from right side
                std::list<Taskmod>::iterator tmp_it = rmod_rit.base ();
                rmods.erase (--tmp_it);
                rmod_rit--;

                // mark this uuid as merged
                uuid_left.insert (uuid);
                lwin = true;
              }
            }
          }
        } // for

        if (rwin)
        {
          DEBUG_STR ("  concat the first match to left branch");
          // concat the oldest (but still newer) modification on the right
          // to the endpoint on the left
          mods.front ().setBefore(tmod_l.getAfter ());
        }
      }
    } // for

    DEBUG_STR ("adding non-conflicting changes from the right branch");
    mods.splice (mods.begin (), rmods);

    DEBUG_STR ("sorting taskmod list");
    mods.sort (compareTaskmod);
    mods_history.sort (compareTaskmod);
  }
  else if (rit == r.end ())
  {
    // nothing happened on the remote branch
    // local branch is up-to-date

    // nothing happened on the local branch either

    // break, to suppress autopush
    if (lit == l.end ())
    {
      mods.clear ();
      lmods.clear ();
      throw std::string (STRING_TDB2_UP_TO_DATE);
    }
  }
  else // lit == l.end ()
  {
    // nothing happened on the local branch
/*
    std::cout << "No local changes detected.\n";
*/

    // add remaining lines (remote branch) to the list of modifications
/*
    std::cout << "Remote changes detected.\n";
*/
    readTaskmods (r, rit, mods);
  }

  ///////////////////////////////////////
  // Now apply the changes.
  // redo command:

  if (!mods.empty ())
  {
    std::vector <std::string> pending_lines;

    std::vector <std::string> completed_lines;

    if (! File::read (pending._file._data, pending_lines))
      throw format (STRING_TDB2_UNREADABLE, pending._file._data);

    if (! File::read (completed._file._data, completed_lines))
      throw format (STRING_TDB2_UNREADABLE, completed._file._data);

    // iterate over taskmod list
    std::list<Taskmod>::iterator it;
    for (it = mods.begin (); it != mods.end (); )
    {
      std::list<Taskmod>::iterator current = it++;
      Taskmod tmod = *current;

      // Modification to an existing task.
      if (!tmod.isNew ())
      {
        std::string uuid = tmod.getUuid ();
        Task::status statusBefore = tmod.getBefore().getStatus ();
        Task::status statusAfter  = tmod.getAfter().getStatus ();

        std::vector <std::string>::iterator it;

        bool found = false;
        if ( (statusBefore == Task::completed)
          || (statusBefore == Task::deleted) )
        {
          // Find the same uuid in completed data
          for (it = completed_lines.begin (); it != completed_lines.end (); ++it)
          {
            if (it->find ("uuid:\"" + uuid) != std::string::npos)
            {
              // Update the completed record.
/*
              std::cout << "Modifying                     "
                        << (context.color () ? colorChanged.colorize (uuid) : uuid)
                        << "\n";
*/

              std::string newline = tmod.getAfter ().composeF4 ();

              // does the tasks move to pending data?
              // this taskmod will not arise from
              // normal usage of task, but those kinds of
              // taskmods may be constructed to merge databases
              if ( (statusAfter != Task::completed)
                && (statusAfter != Task::deleted) )
              {
                // insert task into pending data
                pending_lines.push_back (newline);

                // remove task from completed data
                completed_lines.erase (it);

              }
              else
              {
                // replace the current line
                *it = newline;
              }

              found = true;
              break;
            }
          }
        }
        else
        {
          // Find the same uuid in the pending data.
          for (it = pending_lines.begin (); it != pending_lines.end (); ++it)
          {
            if (it->find ("uuid:\"" + uuid) != std::string::npos)
            {
              // Update the pending record.
              std::cout << format (STRING_TDB2_REMOTE_CHANGE,
                                   (context.color () ? colorChanged.colorize (uuid) : uuid),
                                   cutOff (tmod.getBefore ().get ("description"), 10))
                        << "\n";

              std::string newline = tmod.getAfter ().composeF4 ();

              // does the tasks move to completed data
              if ( (statusAfter == Task::completed)
                || (statusAfter == Task::deleted) )
              {
                // insert task into completed data
                completed_lines.push_back (newline);

                // remove task from pending data
                pending_lines.erase (it);

              }
              else
              {
                // replace the current line
                *it = newline;
              }

              found = true;
              break;
            }
          }
        }

        if (!found)
        {
          std::cout << format (STRING_TDB2_MISSING,
                               (context.color () ? colorRejected.colorize (uuid) : uuid),
                               cutOff (tmod.getBefore ().get ("description"), 10))
                    << "\n";
          mods.erase (current);
        }
      }
      else
      {
        // Check for dups.
        std::string uuid = tmod.getAfter ().get ("uuid");

        // Find the same uuid in the pending data.
        bool found = false;
        std::vector <std::string>::iterator pit;
        for (pit = pending_lines.begin (); pit != pending_lines.end (); ++pit)
        {
          if (pit->find ("uuid:\"" + uuid) != std::string::npos)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          std::cout << format (STRING_TDB2_MERGING,
                               (context.color () ? colorAdded.colorize (uuid) : uuid),
                               cutOff (tmod.getAfter ().get ("description"), 10))
                    << "\n";

          pending_lines.push_back (tmod.getAfter ().composeF4 ());
        }
        else
        {
          mods.erase (current);
        }
      }
    }

    // write pending file
    if (! File::write (pending._file._data, pending_lines))
      throw format (STRING_TDB2_UNWRITABLE, pending._file._data);

    // write completed file
    if (! File::write (completed._file._data, completed_lines))
      throw format (STRING_TDB2_UNWRITABLE, completed._file._data);
  }

  if (!mods.empty() || !lmods.empty() || !mods_history.empty()) {
    // at this point undo contains the lines up to the branch-off point
    // now we merge mods (new modifications from mergefile)
    // with lmods (part of old undo.data)
    lmods.sort(compareTaskmod);
    mods.merge (lmods, compareTaskmod);
    mods.merge (mods_history, compareTaskmod);

    // generate undo.data format
    std::list<Taskmod>::iterator it;
    for (it = mods.begin (); it != mods.end (); it++)
      undo_lines.push_back(it->toString ());

    // write undo file
    if (! File::write (undo._file._data, undo_lines, false))
      throw format (STRING_TDB2_UNWRITABLE, undo._file._data);
  }

  // delete objects
  lmods.clear ();
  mods.clear ();
  mods_history.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert ()
{
  // Extract the details of the last txn, and roll it back.
  std::vector <std::string> u = undo.get_lines ();
  std::string uuid;
  std::string when;
  std::string current;
  std::string prior;
  revert_undo (u, uuid, when, current, prior);

  // Display diff and confirm.
  show_diff (current, prior, when);
  if (! context.config.getBoolean ("confirmation") ||
      confirm (STRING_TDB2_UNDO_CONFIRM))
  {
    // There are six kinds of change possible.  Determine which one, and act
    // accordingly.
    //
    // Revert: task add
    // [1] 0 --> p
    //   - erase from pending
    //   - if in backlog, erase, else cannot undo
    //
    // Revert: task modify
    // [2] p --> p'
    //   - write prior over current in pending
    //   - add prior to backlog
    //
    // Revert: task done/delete
    // [3] p --> c
    //   - add prior to pending
    //   - erase from completed
    //   - add prior to backlog
    //
    // Revert: task modify
    // [4] c --> p
    //   - add prior to completed
    //   - erase from pending
    //   - add prior to backlog
    //
    // Revert: task modify
    // [5] c --> c'
    //   - write prior over current in completed
    //   - add prior to backlog
    //
    // Revert: task log
    // [6] 0 --> c
    //   - erase from completed
    //   - if in backlog, erase, else cannot undo

    // Modify other data files accordingly.
    std::vector <std::string> p = pending.get_lines ();
    revert_pending (p, uuid, current, prior);

    std::vector <std::string> c = completed.get_lines ();
    revert_completed (p, c, uuid, current, prior);

    std::vector <std::string> b = backlog.get_lines ();
    revert_backlog (b, uuid, current, prior);

    // Commit.  If processing makes it this far with no exceptions, then we're
    // done.
    File::write (undo._file._data, u);
    File::write (pending._file._data, p);
    File::write (completed._file._data, c);
    File::write (backlog._file._data, b);
  }
  else
    std::cout << STRING_CMD_CONFIG_NO_CHANGE << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_undo (
  std::vector <std::string>& u,
  std::string& uuid,
  std::string& when,
  std::string& current,
  std::string& prior)
{
  if (u.size () < 3)
    throw std::string (STRING_TDB2_NO_UNDO);

  // pop last tx
  u.pop_back (); // separator.

  current = u.back ().substr (4);
  u.pop_back ();

  if (u.back ().substr (0, 5) == "time ")
  {
    when = u.back ().substr (5);
    u.pop_back ();
    prior = "";
  }
  else
  {
    prior = u.back ().substr (4);
    u.pop_back ();
    when = u.back ().substr (5);
    u.pop_back ();
  }

  // Extract identifying uuid.
  std::string::size_type uuidAtt = current.find ("uuid:\"");
  if (uuidAtt != std::string::npos)
    uuid = current.substr (uuidAtt + 6, 36); // "uuid:"<uuid>" --> <uuid>
  else
    throw std::string (STRING_TDB2_MISSING_UUID);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_pending (
  std::vector <std::string>& p,
  const std::string& uuid,
  const std::string& current,
  const std::string& prior)
{
  std::string uuid_att = "uuid:\"" + uuid + "\"";

  // is 'current' in pending?
  std::vector <std::string>::iterator task;
  for (task = p.begin (); task != p.end (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert - task found in pending.data");

      // Either revert if there was a prior state, or remove the task.
      if (prior != "")
      {
        *task = prior;
        std::cout << STRING_TDB2_REVERTED << "\n";
      }
      else
      {
        p.erase (task);
        std::cout << STRING_TDB2_REMOVED << "\n";
      }

      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_completed (
  std::vector <std::string>& p,
  std::vector <std::string>& c,
  const std::string& uuid,
  const std::string& current,
  const std::string& prior)
{
  std::string uuid_att = "uuid:\"" + uuid + "\"";

  // is 'current' in completed?
  std::vector <std::string>::iterator task;
  for (task = c.begin (); task != c.end (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert_completed - task found in completed.data");

      // Either revert if there was a prior state, or remove the task.
      if (prior != "")
      {
        *task = prior;
        if (task->find ("status:\"pending\"")   != std::string::npos ||
            task->find ("status:\"waiting\"")   != std::string::npos ||
            task->find ("status:\"recurring\"") != std::string::npos)
        {
          c.erase (task);
          p.push_back (prior);
          std::cout << STRING_TDB2_REVERTED << "\n";
          context.debug ("TDB::revert_completed - task belongs in pending.data");
        }
        else
        {
          std::cout << STRING_TDB2_REVERTED << "\n";
          context.debug ("TDB::revert_completed - task belongs in completed.data");
        }
      }
      else
      {
        c.erase (task);

        std::cout << STRING_TDB2_REVERTED << "\n";
        context.debug ("TDB::revert_completed - task removed");
      }

      std::cout << STRING_TDB2_UNDO_COMPLETE << "\n";
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_backlog (
  std::vector <std::string>& b,
  const std::string& uuid,
  const std::string& current,
  const std::string& prior)
{
  std::string uuid_att = "\"uuid\":\"" + uuid + "\"";

  bool found = false;
  std::vector <std::string>::reverse_iterator task;
  for (task = b.rbegin (); task != b.rend (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert_backlog - task found in backlog.data");
      found = true;

      // If this is a new task (no prior), then just remove it from the backlog.
      if (current != "" && prior == "")
      {
        // Yes, this is what is needed, when you want to erase using a reverse
        // iterator.
        b.erase ((++task).base ());
      }

      // If this is a modification of some kind, add the prior to the backlog.
      else
      {
        Task t (prior);
        b.push_back (t.composeJSON ());
      }

      break;
    }
  }

  if (!found)
    throw std::string (STRING_TDB2_UNDO_SYNCED);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::show_diff (
  const std::string& current,
  const std::string& prior,
  const std::string& when)
{
  Date lastChange (strtol (when.c_str (), NULL, 10));

  // Set the colors.
  Color color_red   (context.color () ? context.config.get ("color.undo.before") : "");
  Color color_green (context.color () ? context.config.get ("color.undo.after") : "");

  if (context.config.get ("undo.style") == "side")
  {
    std::cout << "\n"
              << format (STRING_TDB2_LAST_MOD, lastChange.toString ())
              << "\n";

    // Attributes are all there is, so figure the different attribute names
    // between before and after.
    ViewText view;
    view.width (context.getWidth ());
    view.intraPadding (2);
    view.add (Column::factory ("string", ""));
    view.add (Column::factory ("string", STRING_TDB2_UNDO_PRIOR));
    view.add (Column::factory ("string", STRING_TDB2_UNDO_CURRENT));

    Task after (current);

    if (prior != "")
    {
      Task before (prior);

      std::vector <std::string> beforeAtts;
      std::map <std::string, std::string>::iterator att;
      for (att = before.begin (); att != before.end (); ++att)
        beforeAtts.push_back (att->first);

      std::vector <std::string> afterAtts;
      for (att = after.begin (); att != after.end (); ++att)
        afterAtts.push_back (att->first);

      std::vector <std::string> beforeOnly;
      std::vector <std::string> afterOnly;
      listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

      int row;
      std::vector <std::string>::iterator name;
      for (name = beforeOnly.begin (); name != beforeOnly.end (); ++name)
      {
        row = view.addRow ();
        view.set (row, 0, *name);
        view.set (row, 1, renderAttribute (*name, before.get (*name)), color_red);
      }

      for (att = before.begin (); att != before.end (); ++att)
      {
        std::string priorValue   = before.get (att->first);
        std::string currentValue = after.get  (att->first);

        if (currentValue != "")
        {
          row = view.addRow ();
          view.set (row, 0, att->first);
          view.set (row, 1, renderAttribute (att->first, priorValue),
                    (priorValue != currentValue ? color_red : Color ()));
          view.set (row, 2, renderAttribute (att->first, currentValue),
                    (priorValue != currentValue ? color_green : Color ()));
        }
      }

      for (name = afterOnly.begin (); name != afterOnly.end (); ++name)
      {
        row = view.addRow ();
        view.set (row, 0, *name);
        view.set (row, 2, renderAttribute (*name, after.get (*name)), color_green);
      }
    }
    else
    {
      int row;
      std::map <std::string, std::string>::iterator att;
      for (att = after.begin (); att != after.end (); ++att)
      {
        row = view.addRow ();
        view.set (row, 0, att->first);
        view.set (row, 2, renderAttribute (att->first, after.get (att->first)), color_green);
      }
    }

    std::cout << "\n"
              << view.render ()
              << "\n";
  }

  // This style looks like this:
  //  --- before    2009-07-04 00:00:25.000000000 +0200
  //  +++ after    2009-07-04 00:00:45.000000000 +0200
  //
  // - name: old           // att deleted
  // + name:
  //
  // - name: old           // att changed
  // + name: new
  //
  // - name:
  // + name: new           // att added
  //
  else if (context.config.get ("undo.style") == "diff")
  {
    // Create reference tasks.
    Task before;
    if (prior != "")
      before.parse (prior);

    Task after (current);

    // Generate table header.
    ViewText view;
    view.width (context.getWidth ());
    view.intraPadding (2);
    view.add (Column::factory ("string", ""));
    view.add (Column::factory ("string", ""));

    int row = view.addRow ();
    view.set (row, 0, STRING_TDB2_DIFF_PREV, color_red);
    view.set (row, 1, STRING_TDB2_DIFF_PREV_DESC, color_red);

    row = view.addRow ();
    view.set (row, 0, STRING_TDB2_DIFF_CURR, color_green);  // Note trailing space.
    view.set (row, 1, format (STRING_TDB2_DIFF_CURR_DESC,
                              lastChange.toString (context.config.get ("dateformat"))),
                      color_green);

    view.addRow ();

    // Add rows to table showing diffs.
    std::vector <std::string> all = context.getColumns ();

    // Now factor in the annotation attributes.
    Task::iterator it;
    for (it = before.begin (); it != before.end (); ++it)
      if (it->first.substr (0, 11) == "annotation_")
        all.push_back (it->first);

    for (it = after.begin (); it != after.end (); ++it)
      if (it->first.substr (0, 11) == "annotation_")
        all.push_back (it->first);

    // Now render all the attributes.
    std::sort (all.begin (), all.end ());

    std::string before_att;
    std::string after_att;
    std::string last_att;
    std::vector <std::string>::iterator a;
    for (a = all.begin (); a != all.end (); ++a)
    {
      if (*a != last_att)  // Skip duplicates.
      {
        last_att = *a;

        before_att = before.get (*a);
        after_att  = after.get (*a);

        // Don't report different uuid.
        // Show nothing if values are the unchanged.
        if (*a == "uuid" ||
            before_att == after_att)
        {
          // Show nothing - no point displaying that which did not change.

          // row = view.addRow ();
          // view.set (row, 0, *a + ":");
          // view.set (row, 1, before_att);
        }

        // Attribute deleted.
        else if (before_att != "" && after_att == "")
        {
          row = view.addRow ();
          view.set (row, 0, "-" + *a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
        }

        // Attribute added.
        else if (before_att == "" && after_att != "")
        {
          row = view.addRow ();
          view.set (row, 0, "-" + *a + ":", color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }

        // Attribute changed.
        else
        {
          row = view.addRow ();
          view.set (row, 0, "-" + *a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }
      }
    }

    std::cout << "\n"
              << view.render ()
              << "\n";
  }

}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
// Reverts expired waiting tasks to pending.
// Cleans up dangling dependencies.
//
// Possible scenarios:
// - task in pending that needs to be in completed
// - task in completed that needs to be in pending
// - waiting task in pending that needs to be un-waited
int TDB2::gc ()
{
  context.timer_gc.start ();
  unsigned long load_start = context.timer_load.total ();

  // Allowed as an override, but not recommended.
  if (context.config.getBoolean ("gc"))
  {
    std::vector <Task> pending_tasks   = pending.get_tasks ();
    std::vector <Task> completed_tasks = completed.get_tasks ();

    bool pending_changes = false;
    bool completed_changes = false;

    std::vector <Task> pending_tasks_after;
    std::vector <Task> completed_tasks_after;

    // Reduce unnecessary allocation/copies.
    pending_tasks_after.reserve (pending_tasks.size ());
    completed_tasks_after.reserve (completed_tasks.size ());

    // Scan all pending tasks, looking for any that need to be relocated to
    // completed, or need to be 'woken'.
    Date now;
    std::string status;
    std::vector <Task>::iterator task;
    for (task = pending_tasks.begin ();
         task != pending_tasks.end ();
         ++task)
    {
      status = task->get ("status");
      if (status == "pending" ||
          status == "recurring")
      {
        pending_tasks_after.push_back (*task);
      }
      else if (status == "waiting")
      {
        Date wait (task->get_date ("wait"));
        if (wait < now)
        {
          task->set ("status", "pending");
          task->remove ("wait");
          pending_changes = true;
        }

        pending_tasks_after.push_back (*task);
      }
      else
      {
        completed_tasks_after.push_back (*task);
        pending_changes = true;
        completed_changes = true;
      }
    }

    // Scan all completed tasks, looking for any that need to be relocated to
    // pending.
    for (task = completed_tasks.begin ();
         task != completed_tasks.end ();
         ++task)
    {
      status = task->get ("status");
      if (status == "pending" ||
          status == "recurring")
      {
        pending_tasks_after.push_back (*task);
        pending_changes = true;
        completed_changes = true;
      }
      else if (status == "waiting")
      {
        Date wait (task->get_date ("wait"));
        if (wait < now)
        {
          task->set ("status", "pending");
          task->remove ("wait");
          pending_tasks_after.push_back (*task);
          pending_changes = true;
          completed_changes = true;
        }

        pending_tasks_after.push_back (*task);
      }
      else
      {
        completed_tasks_after.push_back (*task);
      }
    }

    // Only recreate the pending.data file if necessary.
    if (pending_changes)
    {
      pending._tasks = pending_tasks_after;
      pending._dirty = true;
      pending._loaded_tasks = true;
      _id = 1;

      for (task = pending._tasks.begin ();
           task != pending._tasks.end ();
           ++task)
      {
        task->id = _id++;
      }

      // Note: deliberately no commit.
    }

    // Only recreate the completed.data file if necessary.
    if (completed_changes)
    {
      completed._tasks = completed_tasks_after;
      completed._dirty = true;
      completed._loaded_tasks = true;

      // Note: deliberately no commit.
    }

    // TODO Remove dangling dependencies
  }

  // Stop and remove accumulated load time from the GC time, because they
  // overlap.
  context.timer_gc.stop ();
  context.timer_gc.subtract (context.timer_load.total () - load_start);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Next ID is that of the last pending task plus one.
int TDB2::next_id ()
{
  return _id++;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::all_tasks ()
{
  std::vector <Task> all (pending._tasks.size () +
                          pending._added_tasks.size () +
                          completed._tasks.size () +
                          completed._added_tasks.size ());
  all = pending.get_tasks ();

  std::vector <Task> extra (completed._tasks.size () +
                            completed._added_tasks.size ());
  extra = completed.get_tasks ();

  std::vector <Task>::iterator task;
  for (task = extra.begin (); task != extra.end (); ++task)
    all.push_back (*task);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by ID, wherever it is.
bool TDB2::get (int id, Task& task)
{
  return pending.get   (id, task) ||
         completed.get (id, task);
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID, wherever it is.
bool TDB2::get (const std::string& uuid, Task& task)
{
  return pending.get   (uuid, task) ||
         completed.get (uuid, task);
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::siblings (Task& task)
{
  std::vector <Task> results;
  if (task.has ("parent"))
  {
    std::string parent = task.get ("parent");

    // First load and scan pending.
    if (! pending._loaded_tasks)
      pending.load_tasks ();

    std::vector <Task>::iterator i;
    for (i = pending._tasks.begin (); i != pending._tasks.end (); ++i)
    {
      // Do not include self in results.
      if (i->id != task.id)
      {
        // Do not include completed or deleted tasks.
        if (i->getStatus () != Task::completed &&
            i->getStatus () != Task::deleted)
        {
          // If task has the same parent, it is a sibling.
          if (i->has ("parent") &&
              i->get ("parent") == parent)
          {
            results.push_back (*i);
          }
        }
      }
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::children (Task& task)
{
  std::vector <Task> results;
  std::string parent = task.get ("uuid");

  // First load and scan pending.
  if (! pending._loaded_tasks)
    pending.load_tasks ();

  std::vector <Task>::iterator i;
  for (i = pending._tasks.begin (); i != pending._tasks.end (); ++i)
  {
    // Do not include self in results.
    if (i->id != task.id)
    {
      // Do not include completed or deleted tasks.
      if (i->getStatus () != Task::completed &&
          i->getStatus () != Task::deleted)
      {
        // If task has the same parent, it is a sibling.
        if (i->get ("parent") == parent)
          results.push_back (*i);
      }
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
std::string TDB2::uuid (int id)
{
  std::string result = pending.uuid (id);
  if (result == "")
    result = completed.uuid (id);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::id (const std::string& uuid)
{
  int result = pending.id (uuid);
  if (result == 0)
    result = completed.id (uuid);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Make sure the specified UUID does not already exist in the data.
bool TDB2::verifyUniqueUUID (const std::string& uuid)
{
  pending.get_tasks ();
  return pending.id (uuid) != 0 ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB2::read_only ()
{
  return pending._read_only   ||
         completed._read_only ||
         undo._read_only      ||
         backlog._read_only
         ;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::clear ()
{
  pending.clear ();
  completed.clear ();
  undo.clear ();
  backlog.clear ();

  _location = "";
  _id = 1;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::dump ()
{
  if (context.config.getBoolean ("debug"))
  {
    context.debug (pending.dump ());
    context.debug (completed.dump ());
    context.debug (undo.dump ());
    context.debug (backlog.dump ());
    context.debug ("");
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
