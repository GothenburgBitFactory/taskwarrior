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
#ifndef INCLUDED_MAIN
#define INCLUDED_MAIN

#define FEATURE_TDB_OPT      1   // TDB Optimization reduces file I/O.
#define FEATURE_NEW_ID       1   // Echoes back new id.
#define FEATURE_SHELL        1   // Interactive shell.
#define FEATURE_NCURSES_COLS 1   // Shortcut that avoids WINDOW.

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include "Context.h"
#include "Table.h"
#include "Date.h"
#include "Color.h"
#include "../auto.h"

// task.cpp
void gatherNextTasks (std::vector <Task>&);
void onChangeCallback ();

// recur.cpp
void handleRecurrence ();
Date getNextRecurrence (Date&, std::string&);
bool generateDueDates (Task&, std::vector <Date>&);
void updateRecurrenceMask (std::vector <Task>&, Task&);
int getDueState (const std::string&);
bool nag (Task&);

// command.cpp
int handleAdd (std::string &);
int handleAppend (std::string &);
int handlePrepend (std::string &);
int handleExport (std::string &);
int handleDone (std::string &);
int handleModify (std::string &);
int handleProjects (std::string &);
int handleCompletionProjects (std::string &);
int handleTags (std::string &);
int handleCompletionTags (std::string &);
int handleCompletionCommands (std::string &);
int handleCompletionIDs (std::string &);
int handleCompletionConfig (std::string &);
int handleVersion (std::string &);
int handleDelete (std::string &);
int handleStart (std::string &);
int handleStop (std::string &);
int handleColor (std::string &);
int handleAnnotate (std::string &);
int handleDuplicate (std::string &);
void handleUndo ();
#ifdef FEATURE_SHELL
void handleShell ();
#endif
int deltaAppend (Task&);
int deltaPrepend (Task&);
int deltaDescription (Task&);
int deltaTags (Task&);
int deltaAttributes (Task&);
int deltaSubstitutions (Task&);

// edit.cpp
int handleEdit (std::string &);

// report.cpp
int shortUsage (std::string &);
int longUsage (std::string &);
int handleInfo (std::string &);
int handleReportSummary (std::string &);
int handleReportNext (std::string &);
int handleReportHistory (std::string &);
int handleReportGHistory (std::string &);
int handleReportCalendar (std::string &);
int handleReportStats (std::string &);
int handleReportTimesheet (std::string &);
std::string getFullDescription (Task&);
std::string getDueDate (Task&);

// custom.cpp
int handleCustomReport (const std::string&, std::string &);
int runCustomReport (const std::string&, const std::string&,
                     const std::string&, const std::string&,
                     const std::string&, std::vector <Task>&,
		     std::string&);
void validReportColumns (const std::vector <std::string>&);
void validSortColumns (const std::vector <std::string>&, const std::vector <std::string>&);

// rules.cpp
void initializeColorRules ();
void autoColorize (Task&, Color&);
std::string colorizeHeader (const std::string&);
std::string colorizeMessage (const std::string&);
std::string colorizeFootnote (const std::string&);
std::string colorizeDebug (const std::string&);

// import.cpp
int handleImport (std::string&);

// list template
///////////////////////////////////////////////////////////////////////////////
template <class T> bool listDiff (const T& left, const T& right)
{
  if (left.size () != right.size ())
    return true;

  for (unsigned int i = 0; i < left.size (); ++i)
    if (left[i] != right[i])
      return true;

  return false;
}

///////////////////////////////////////////////////////////////////////////////
template <class T> void listDiff (
  const T& left, const T& right, T& leftOnly, T& rightOnly)
{
  leftOnly.clear ();
  rightOnly.clear ();

  for (unsigned int l = 0; l < left.size (); ++l)
  {
    bool found = false;
    for (unsigned int r = 0; r < right.size (); ++r)
    {
      if (left[l] == right[r])
      {
        found = true;
        break;
      }
    }

    if (!found)
      leftOnly.push_back (left[l]);
  }

  for (unsigned int r = 0; r < right.size (); ++r)
  {
    bool found = false;
    for (unsigned int l = 0; l < left.size (); ++l)
    {
      if (left[l] == right[r])
      {
        found = true;
        break;
      }
    }

    if (!found)
      rightOnly.push_back (right[r]);
  }
}

////////////////////////////////////////////////////////////////////////////////
template <class T> void listIntersect (const T& left, const T& right, T& join)
{
  join.clear ();

  for (unsigned int l = 0; l < left.size (); ++l)
  {
    for (unsigned int r = 0; r < right.size (); ++r)
    {
      if (left[l] == right[r])
      {
        join.push_back (left[l]);
        break;
      }
    }
  }
}

#endif
////////////////////////////////////////////////////////////////////////////////
