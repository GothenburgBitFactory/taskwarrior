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
#ifndef INCLUDED_T2
#define INCLUDED_T2

#include <string>
#include "Record.h"
#include "Subst.h"
#include "Sequence.h"

class T2 : public Record
{
public:
  T2 ();                     // Default constructor
  T2 (const std::string&);   // Parse
  T2& operator= (const T2&); // Assignment operator
  ~T2 ();                    // Destructor

  std::string composeCSV ();

  // Status values.
  enum status {pending, completed, deleted, recurring /* , retired, deferred */};

  // Public data.
  Sequence sequence;
  Subst    subst;

  // Series of helper functions.
  int id () const                                      { return sequence.size () ? sequence[0] : 0; }
  void id (int anotherId)                              { sequence.push_back (anotherId); }

/*
  status getStatus () const                            { return mStatus; }
  void setStatus (status s)                            { mStatus = s; }

  bool hasTag (const std::string&) const;
  void getRemoveTags (std::vector<std::string>&);      // SPECIAL
  void addRemoveTag (const std::string&);              // SPECIAL
  int getTagCount () const;
  void getTags (std::vector<std::string>&) const;
*/
  void addTag (const std::string&);
/*
  void addTags (const std::vector <std::string>&);
*/
  void removeTag (const std::string&);
/*
  void removeTags ();
*/

  void getAnnotations (std::vector <Att>&) const;
  void setAnnotations (const std::vector <Att>&);
  void addAnnotation (const std::string&);

  bool validate () const;

private:
  int determineVersion (const std::string&);
  void legacyParse (const std::string&);

private:
/*
  std::vector<std::string>           mTags;
  std::vector<std::string>           mRemoveTags;
*/
};

#endif
////////////////////////////////////////////////////////////////////////////////
