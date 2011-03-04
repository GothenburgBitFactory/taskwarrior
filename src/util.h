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
#ifndef INCLUDED_UTIL
#define INCLUDED_UTIL

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include "Task.h"
#include "../cmake.h"

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

// util.cpp
bool confirm (const std::string&);
int confirm3 (const std::string&);
int confirm4 (const std::string&);
void delay (float);
std::string formatBytes (size_t);
int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&);
const std::string uuid ();

#ifdef SOLARIS
  #define LOCK_SH 1
  #define LOCK_EX 2
  #define LOCK_NB 4
  #define LOCK_UN 8

  int flock (int, int);
#endif

bool taskDiff (const Task&, const Task&);
std::string taskDifferences (const Task&, const Task&);
std::string taskInfoDifferences (const Task&, const Task&);
std::string renderAttribute (const std::string&, const std::string&);

#endif
////////////////////////////////////////////////////////////////////////////////
