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
#ifndef INCLUDED_REPORTSTATS
#define INCLUDED_REPORTSTATS

#include <map>
#include <string>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Element.h"

class ReportStats : public Element
{
public:
  ReportStats ();
  ReportStats (const ReportStats&);
  ReportStats& operator= (const ReportStats&);
  ~ReportStats ();

  void initialize ();
  bool dataChanged ();
  void gatherStats ();
  bool event (int);
  void redraw ();
  void renderItem (int, int, const std::string&, const std::string&);

public:
  struct stat stat_pending;
  struct stat stat_completed;
  struct stat stat_undo;

private:
  pthread_t ticker;
  std::map <std::string, std::string> previous;
  std::map <std::string, std::string> current;
  bool highlight_shown;
};

#endif
////////////////////////////////////////////////////////////////////////////////

