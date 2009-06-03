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

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include "Context.h"
#include "T.h"
#include "TDB.h"
#include "Table.h"
#include "Date.h"
#include "color.h"
#include "../auto.h"

// parse.cpp
void parse (std::vector <std::string>&, std::string&, T&);
bool validPriority (const std::string&);
bool validDate (std::string&);
bool validDuration (std::string&);
void loadCustomReports ();
bool isCustomReport (const std::string&);
void allCustomReports (std::vector <std::string>&);

// task.cpp
void gatherNextTasks (const TDB&, T&, std::vector <T>&, std::vector <int>&);
void nag (TDB&, T&);
int getDueState (const std::string&);
void handleRecurrence (TDB&, std::vector <T>&);
bool generateDueDates (T&, std::vector <Date>&);
Date getNextRecurrence (Date&, std::string&);
void updateRecurrenceMask (TDB&, std::vector <T>&, T&);
void onChangeCallback ();
std::string runTaskCommand (int, char**, TDB&, bool gc = true, bool shadow = true);
std::string runTaskCommand (std::vector <std::string>&, TDB&, bool gc = false, bool shadow = false);

// command.cpp
std::string handleAdd (TDB&, T&);
std::string handleAppend (TDB&, T&);
std::string handleExport (TDB&, T&);
std::string handleDone (TDB&, T&);
std::string handleModify (TDB&, T&);
std::string handleProjects (TDB&, T&);
std::string handleTags (TDB&, T&);
std::string handleUndelete (TDB&, T&);
std::string handleVersion ();
std::string handleDelete (TDB&, T&);
std::string handleStart (TDB&, T&);
std::string handleStop (TDB&, T&);
std::string handleUndo (TDB&, T&);
std::string handleColor ();
std::string handleAnnotate (TDB&, T&);
std::string handleDuplicate (TDB&, T&);
T findT (int, const std::vector <T>&);
int deltaAppend (T&, T&);
int deltaDescription (T&, T&);
int deltaTags (T&, T&);
int deltaAttributes (T&, T&);
int deltaSubstitutions (T&, T&);

// edit.cpp
std::string handleEdit (TDB&, T&);

// report.cpp
void filterSequence (std::vector<T>&, T&);
void filter (std::vector<T>&, T&);
std::string handleInfo (TDB&, T&);
std::string handleCompleted (TDB&, T&);
std::string handleReportSummary (TDB&, T&);
std::string handleReportNext (TDB&, T&);
std::string handleReportHistory (TDB&, T&);
std::string handleReportGHistory (TDB&, T&);
std::string handleReportCalendar (TDB&, T&);
std::string handleReportActive (TDB&, T&);
std::string handleReportOverdue (TDB&, T&);
std::string handleReportStats (TDB&, T&);
std::string handleReportTimesheet (TDB&, T&);

std::string handleCustomReport (TDB&, T&, const std::string&);
void validReportColumns (const std::vector <std::string>&);
void validSortColumns (const std::vector <std::string>&, const std::vector <std::string>&);

// rules.cpp
void initializeColorRules ();
void autoColorize (T&, Text::color&, Text::color&);

// import.cpp
std::string handleImport (TDB&, T&);

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
