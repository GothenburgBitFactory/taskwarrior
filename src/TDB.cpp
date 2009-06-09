////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>

#include "task.h"
#include "TDB.h"

////////////////////////////////////////////////////////////////////////////////
TDB::TDB ()
: mPendingFile ("")
, mCompletedFile ("")
, mId (1)
, mNoLock (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TDB::~TDB ()
{
}

////////////////////////////////////////////////////////////////////////////////
void TDB::dataDirectory (const std::string& directory)
{
  if (! access (expandPath (directory).c_str (), F_OK))
  {
    mPendingFile   = directory + "/pending.data";
    mCompletedFile = directory + "/completed.data";
  }
  else
  {
    std::string error = "Directory '";
    error += directory;
    error += "' does not exist, or is not readable and writable.";
    throw error;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Combine allPendingT with allCompletedT.
// Note: this method is O(N1) + O(N2), where N2 is not bounded.
bool TDB::allT (std::vector <Tt>& all)
{
  all.clear ();

  // Retrieve all the pending records.
  std::vector <Tt> allp;
  if (allPendingT (allp))
  {
    std::vector <Tt>::iterator i;
    for (i = allp.begin (); i != allp.end (); ++i)
      all.push_back (*i);

    // Retrieve all the completed records.
    std::vector <Tt> allc;
    if (allCompletedT (allc))
    {
      for (i = allc.begin (); i != allc.end (); ++i)
        all.push_back (*i);

      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Only accesses to the pending file result in Tasks that have assigned ids.
bool TDB::pendingT (std::vector <Tt>& all)
{
  all.clear ();

  std::vector <std::string> lines;
  if (readLockedFile (mPendingFile, lines))
  {
    mId = 1;

    int line = 1;
    std::vector <std::string>::iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      try
      {
        Tt t (*it);
        t.setId (mId++);
        if (t.getStatus () == Tt::pending)
          all.push_back (t);
      }

      catch (std::string& e)
      {
        std::stringstream more;
        more << "  Line " << line << ", in " << "pending.data";

        throw e + more.str ();
      }

      ++line;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Only accesses to the pending file result in Tasks that have assigned ids.
bool TDB::allPendingT (std::vector <Tt>& all)
{
  all.clear ();

  std::vector <std::string> lines;
  if (readLockedFile (mPendingFile, lines))
  {
    mId = 1;

    int line = 1;
    std::vector <std::string>::iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      try
      {
        Tt t (*it);
        t.setId (mId++);
        all.push_back (t);
      }

      catch (std::string& e)
      {
        std::stringstream more;
        more << "  Line " << line << ", in " << "pending.data";

        throw e + more.str ();
      }

      ++line;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::completedT (std::vector <Tt>& all) const
{
  all.clear ();

  std::vector <std::string> lines;
  if (readLockedFile (mCompletedFile, lines))
  {
    int line = 1;
    std::vector <std::string>::iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      try
      {
        Tt t (*it);
        if (t.getStatus () != Tt::deleted)
          all.push_back (t);
      }

      catch (std::string& e)
      {
        std::stringstream more;
        more << "  Line " << line << ", in " << "pending.data";

        throw e + more.str ();
      }

      ++line;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::allCompletedT (std::vector <Tt>& all) const
{
  all.clear ();

  std::vector <std::string> lines;
  if (readLockedFile (mCompletedFile, lines))
  {
    int line = 1;
    std::vector <std::string>::iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      try
      {
        Tt t (*it);
        all.push_back (t);
      }

      catch (std::string& e)
      {
        std::stringstream more;
        more << "  Line " << line << ", in " << "pending.data";

        throw e + more.str ();
      }

      ++line;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::addT (const Tt& t)
{
  Tt task (t);
  std::vector <std::string> tags;
  task.getTags (tags);

  // +tag or -tag are both considered valid tags to add to a new pending task.
  // Generating an error here would not be friendly.
  for (unsigned int i = 0; i < tags.size (); ++i)
  {
    if (tags[i][0] == '-' || tags[i][0] == '+')
    {
      task.removeTag (tags[i]);
      task.addTag (tags[i].substr (1, std::string::npos));
    }
  }

  if (task.getStatus () == Tt::pending ||
      task.getStatus () == Tt::recurring)
  {
    return writePending (task);
  }

  return writeCompleted (task);
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::modifyT (const Tt& t)
{
  Tt modified (t);

  std::vector <Tt> all;
  allPendingT (all);

  std::vector <Tt> pending;

  std::vector <Tt>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == t.getId ())
    {
      modified.setUUID (it->getUUID ());
      pending.push_back (modified);
    }
    else
      pending.push_back (*it);
  }

  return overwritePending (pending);
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::lock (FILE* file) const
{
  if (mNoLock)
    return true;

  return flock (fileno (file), LOCK_EX) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::overwritePending (std::vector <Tt>& all)
{
  // Write a single task to the pending file
  FILE* out;
  if ((out = fopen (mPendingFile.c_str (), "w")))
  {
    int retry = 0;
    if (!mNoLock)
      while (flock (fileno (out), LOCK_EX) && ++retry <= 3)
        delay (0.1);

    std::vector <Tt>::iterator it;
    for (it = all.begin (); it != all.end (); ++it)
      fputs (it->compose ().c_str (), out);

    fclose (out);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::writePending (const Tt& t)
{
  // Write a single task to the pending file
  FILE* out;
  if ((out = fopen (mPendingFile.c_str (), "a")))
  {
    int retry = 0;
    if (!mNoLock)
      while (flock (fileno (out), LOCK_EX) && ++retry <= 3)
        delay (0.1);

    fputs (t.compose ().c_str (), out);

    fclose (out);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::writeCompleted (const Tt& t)
{
  // Write a single task to the pending file
  FILE* out;
  if ((out = fopen (mCompletedFile.c_str (), "a")))
  {
    int retry = 0;
    if (!mNoLock)
      while (flock (fileno (out), LOCK_EX) && ++retry <= 3)
        delay (0.1);

    fputs (t.compose ().c_str (), out);

    fclose (out);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::readLockedFile (
  const std::string& file,
  std::vector <std::string>& contents) const
{
  contents.clear ();

  if (! access (file.c_str (), F_OK | R_OK))
  {
    FILE* in;
    if ((in = fopen (file.c_str (), "r")))
    {
      int retry = 0;
      if (!mNoLock)
        while (flock (fileno (in), LOCK_EX) && ++retry <= 3)
          delay (0.1);

      char line[T_LINE_MAX];
      while (fgets (line, T_LINE_MAX, in))
      {
        int length = ::strlen (line);
        if (length > 1)
        {
          line[length - 1] = '\0'; // Kill \n
          contents.push_back (line);
        }
      }

      fclose (in);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
int TDB::gc ()
{
  int count = 0;

  // Read everything from the pending file.
  std::vector <Tt> all;
  allPendingT (all);

  // A list of the truly pending tasks.
  std::vector <Tt> pending;

  std::vector<Tt>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    // Some tasks stay in the pending file.
    if (it->getStatus () == Tt::pending ||
        it->getStatus () == Tt::recurring)
    {
      pending.push_back (*it);
    }

    // Others are transferred to the completed file.
    else
    {
      writeCompleted (*it);
      ++count;
    }
  }

  // Dump all clean tasks into pending.  But don't bother unless at least one
  // task was transferred.
  if (count)
    overwritePending (pending);

  return count;
}

////////////////////////////////////////////////////////////////////////////////
int TDB::nextId ()
{
  return mId++;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::noLock ()
{
  mNoLock = true;
}

////////////////////////////////////////////////////////////////////////////////

