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
#ifndef INCLUDED_MAIN
#define INCLUDED_MAIN
#define L10N                                           // Localization complete.

#define FEATURE_NEW_ID       1   // Echoes back new id.
//#define FEATURE_REGEX        1   // Enables regexes for attribute modifiers,
//                                 // subst, general search.

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <Context.h>
#include <Date.h>
#include <Color.h>
#include <cmake.h>

// recur.cpp
void handleRecurrence ();
Date getNextRecurrence (Date&, std::string&);
bool generateDueDates (Task&, std::vector <Date>&);
void updateRecurrenceMask (std::vector <Task>&, Task&);
int getDueState (const std::string&);
bool nag (Task&);

// command.cpp
int handleAdd (std::string&);
int handleLog (std::string&);
int handleAppend (std::string&);
int handlePrepend (std::string&);
int handleDone (std::string&);
int handleModify (std::string&);
int handleProjects (std::string&);
int handleCompletionProjects (std::string&);
int handleCompletionTags (std::string&);
int handleCompletionCommands (std::string&);
int handleCompletionIDs (std::string&);
int handleCompletionConfig (std::string&);
int handleUrgency (std::string&);
int handleQuery (std::string&);
int handleZshCompletionCommands (std::string&);
int handleZshCompletionIDs (std::string&);
int handleConfig (std::string&);
int handleDelete (std::string&);
int handleStart (std::string&);
int handleStop (std::string&);
int handleColor (std::string&);
int handleAnnotate (std::string&);
int handleDenotate (std::string&);
int handleDuplicate (std::string&);
int handleCount (std::string&);
int handleIds (std::string&);
void handleUndo ();
void handleMerge (std::string&);
void handlePush (std::string&);
void handlePull (std::string&);
void handleShell ();
int deltaAppend (Task&);
int deltaPrepend (Task&);
int deltaDescription (Task&);
int deltaTags (Task&);
int deltaAttributes (Task&);
int deltaSubstitutions (Task&);

// report.cpp
int handleReportSummary (std::string&);
int handleReportCalendar (std::string&);
int handleReportStats (std::string&);
int handleReportTimesheet (std::string&);
std::string getFullDescription (Task&, const std::string&);
std::string getDueDate (Task&, const std::string&);
std::string onProjectChange (Task&, bool scope = true);
std::string onProjectChange (Task&, Task&);

// burndown.cpp
int handleReportBurndownDaily (std::string&);
int handleReportBurndownWeekly (std::string&);
int handleReportBurndownMonthly (std::string&);

// history.cpp
int handleReportHistoryMonthly (std::string&);
int handleReportHistoryAnnual (std::string&);
int handleReportGHistoryMonthly (std::string&);
int handleReportGHistoryAnnual (std::string&);

// rules.cpp
void initializeColorRules ();
void autoColorize (Task&, Color&);
std::string colorizeHeader (const std::string&);
std::string colorizeFootnote (const std::string&);
std::string colorizeDebug (const std::string&);

// import.cpp
int handleImport (std::string&);

// export.cpp
int handleExportCSV (std::string&);
int handleExportiCal (std::string&);
int handleExportYAML (std::string&);

// dependency.cpp
bool dependencyIsBlocked (const Task&);
void dependencyGetBlocked (const Task&, std::vector <Task>&);
bool dependencyIsBlocking (const Task&);
void dependencyGetBlocking (const Task&, std::vector <Task>&);
bool dependencyIsCircular (const Task&);
void dependencyChainOnComplete (Task&);
void dependencyChainOnStart (Task&);
void dependencyChainOnModify (Task&, Task&);

// feedback.cpp
bool taskDiff (const Task&, const Task&);
std::string taskDifferences (const Task&, const Task&);
std::string taskInfoDifferences (const Task&, const Task&);
std::string renderAttribute (const std::string&, const std::string&);
std::string feedback (const Task&, const Task&);

// sort.cpp
void sort_tasks (std::vector <Task>&, std::vector <int>&, const std::string&);

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
