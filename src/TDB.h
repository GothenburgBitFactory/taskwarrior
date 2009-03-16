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
#ifndef INCLUDED_TDB
#define INCLUDED_TDB

#include <vector>
#include <string>
#include "T.h"

class TDB
{
public:
  TDB ();
  ~TDB ();

  void dataDirectory    (const std::string&);
  bool allT             (std::vector <T>&);
  bool pendingT         (std::vector <T>&);
  bool allPendingT      (std::vector <T>&);
  bool completedT       (std::vector <T>&) const;
  bool allCompletedT    (std::vector <T>&) const;
  bool deleteT          (const T&);
  bool completeT        (const T&);
  bool addT             (const T&);
  bool modifyT          (const T&);
  bool logRead          (std::vector <std::string>&) const;
  int gc                ();
  int nextId            ();

  void noLock           ();

private:
  bool lock             (FILE*) const;
  bool overwritePending (std::vector <T>&);
  bool writePending     (const T&);
  bool writeCompleted   (const T&);
  bool readLockedFile   (const std::string&, std::vector <std::string>&) const;

private:
  std::string mPendingFile;
  std::string mCompletedFile;
  int mId;
  bool mNoLock;
};

#endif
////////////////////////////////////////////////////////////////////////////////
