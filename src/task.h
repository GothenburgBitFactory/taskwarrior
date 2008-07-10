////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include "Config.h"
#include "Table.h"
#include "Date.h"
#include "color.h"
#include "TDB.h"
#include "T.h"
#include "../auto.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define foreach(i, c)                                              \
for (typeof (c) *foreach_p = & (c);                                \
     foreach_p;                                                    \
     foreach_p = 0)                                                \
  for (typeof (foreach_p->begin()) i = foreach_p->begin();         \
       i != foreach_p->end();                                      \
       ++i)

// parse.cpp
void parse (std::vector <std::string>&, std::string&, T&, Config&);
bool validDate (std::string&, Config&);

// task.cpp
void gatherNextTasks (const TDB&, T&, Config&, std::vector <T>&, std::vector <int>&);
void nag (TDB&, T&, Config&);
int getDueState (const std::string&);
void handleRecurrence (TDB&, std::vector <T>&);
void generateDueDates (T&, std::vector <Date>&);
Date getNextRecurrence (Date&, std::string&);
void updateRecurrenceMask (TDB&, std::vector <T>&, T&);

// command.cpp
void handleAdd (const TDB&, T&, Config&);
void handleProjects (TDB&, T&, Config&);
void handleTags (TDB&, T&, Config&);
void handleUndelete (TDB&, T&, Config&);
void handleVersion (Config&);
void handleExport (TDB&, T&, Config&);
void handleDelete (TDB&, T&, Config&);
void handleStart (TDB&, T&, Config&);
void handleDone (TDB&, T&, Config&);
void handleModify (TDB&, T&, Config&);
void handleColor (Config&);

// report.cpp
void filter (std::vector<T>&, T&);
void handleList (TDB&, T&, Config&);
void handleInfo (TDB&, T&, Config&);
void handleLongList (TDB&, T&, Config&);
void handleSmallList (TDB&, T&, Config&);
void handleCompleted (TDB&, T&, Config&);
void handleReportSummary (TDB&, T&, Config&);
void handleReportNext (TDB&, T&, Config&);
void handleReportHistory (TDB&, T&, Config&);
void handleReportGHistory (TDB&, T&, Config&);
void handleReportUsage (const TDB&, T&, Config&);
void handleReportCalendar (TDB&, T&, Config&);
void handleReportActive (TDB&, T&, Config&);
void handleReportOverdue (TDB&, T&, Config&);
void handleReportStats (TDB&, T&, Config&);
void handleReportOldest (TDB&, T&, Config&);
void handleReportNewest (TDB&, T&, Config&);

// util.cpp
bool confirm (const std::string&);
void wrapText (std::vector <std::string>&, const std::string&, const int);
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
void extractParagraphs (const std::string&, std::vector<std::string>&);
void extractLine (std::string&, std::string&, int);
void split (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const std::string&);
void join (std::string&, const std::string&, const std::vector<std::string>&);
std::string commify (const std::string&);
std::string lowerCase (const std::string&);
std::string upperCase (const std::string&);
void delay (float);
int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&);
void formatTimeDeltaDays (std::string&, time_t);
std::string formatSeconds (time_t);
const std::string uuid ();
const char* optionalBlankLine (Config&);
int convertDuration (std::string&);
std::string expandPath (const std::string&);

// rules.cpp
void initializeColorRules (Config&);
void autoColorize (T&, Text::color&, Text::color&);

////////////////////////////////////////////////////////////////////////////////
