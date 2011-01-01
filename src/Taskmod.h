////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Johannes Schlatow.
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
#ifndef INCLUDED_TASKMOD
#define INCLUDED_TASKMOD

#include <string>
#include <Task.h>

class Taskmod {
public:
  Taskmod ();
  Taskmod (const Taskmod& other);
  ~Taskmod ();

  // operators
  bool operator< (const Taskmod& compare);
  bool operator> (const Taskmod& compare);
  bool operator== (const Taskmod& compare);
  bool operator!= (const Taskmod& compare);
  Taskmod& operator= (const Taskmod& other);

  // helper
  void reset (long timestamp=0);
  bool isNew ();
  bool issetBefore ();
  bool issetAfter ();
  bool isValid ();

  std::string getUuid ();
  std::string toString ();

  // setter
  void setAfter (const Task& after);
  void setBefore (const Task& before);
  void setTimestamp (long timestamp);

  // getter
  Task& getAfter ();
  Task& getBefore ();
  long getTimestamp () const;
  std::string getTimeStr () const;

protected:
  Task after;
  Task before;
  long timestamp;
  bool bAfterSet;
  bool bBeforeSet;
};

#endif

