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

// TDB Optimization attempts to reduce the amount of I/O.
#define FEATURE_TDB_OPT 1


#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include "Context.h"
#include "Table.h"
#include "Date.h"
#include "color.h"
#include "../auto.h"

// valid.cpp
void validReportColumns (const std::vector <std::string>&);
void validSortColumns (const std::vector <std::string>&, const std::vector <std::string>&);
bool validTag (const std::string&);

// task.cpp
void gatherNextTasks (std::vector <Task>&, std::vector <int>&);
void onChangeCallback ();

// recur.cpp
void handleRecurrence (std::vector <Task>&);
Date getNextRecurrence (Date&, std::string&);
bool generateDueDates (Task&, std::vector <Date>&);
void updateRecurrenceMask (std::vector <Task>&, Task&);
int getDueState (const std::string&);
void nag (Task&);

// command.cpp
std::string handleAdd ();
std::string handleAppend ();
std::string handleExport ();
std::string handleDone ();
std::string handleModify ();
std::string handleProjects ();
std::string handleTags ();
std::string handleUndelete ();
std::string handleVersion ();
std::string handleDelete ();
std::string handleStart ();
std::string handleStop ();
std::string handleUndo ();
std::string handleColor ();
std::string handleAnnotate ();
std::string handleDuplicate ();
int deltaAppend (Task&);
int deltaDescription (Task&);
int deltaTags (Task&);
int deltaAttributes (Task&);
int deltaSubstitutions (Task&);

// edit.cpp
std::string handleEdit ();

// report.cpp
std::string shortUsage ();
std::string longUsage ();
std::string handleInfo ();
std::string handleReportSummary ();
std::string handleReportNext ();
std::string handleReportHistory ();
std::string handleReportGHistory ();
std::string handleReportCalendar ();
std::string handleReportStats ();
std::string handleReportTimesheet ();
std::string getFullDescription (Task&);
std::string getDueDate (Task&);

// custom.cpp
std::string handleCustomReport (const std::string&);

// rules.cpp
void initializeColorRules ();
void autoColorize (Task&, Text::color&, Text::color&);
std::string colorizeHeader (const std::string&);
std::string colorizeMessage (const std::string&);
std::string colorizeFootnote (const std::string&);

// import.cpp
std::string handleImport ();

// list template
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
