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
#ifndef INCLUDED_TASK
#define INCLUDED_TASK
#define L10N                                           // Localization complete.

#include <string>
#include <Record.h>

class Task : public Record
{
public:
  Task ();                       // Default constructor
  Task (const Task&);            // Copy constructor
  Task& operator= (const Task&); // Assignment operator
  bool operator== (const Task&); // Comparison operator
  Task (const std::string&);     // Parse
  ~Task ();                      // Destructor

  void parse (const std::string&);
  std::string composeF4 () const;
  std::string composeCSV () const;
  std::string composeYAML () const;
  std::string composeJSON (bool include_id = false) const;

  // Status values.
  enum status {pending, completed, deleted, recurring, waiting};

  // Public data.
  int id;
  float urgency_value;
  bool recalc_urgency;

  // Series of helper functions.
  static status textToStatus (const std::string&);
  static std::string statusToText (status);

  void setEntry ();
  void setEnd ();
  void setStart ();

  status getStatus () const;
  void setStatus (status);

  int getTagCount () const;
  bool hasTag (const std::string&) const;
  void addTag (const std::string&);
  void addTags (const std::vector <std::string>&);
  void getTags (std::vector<std::string>&) const;
  void removeTag (const std::string&);

  void getAnnotations (std::vector <Att>&) const;
  void setAnnotations (const std::vector <Att>&);
  void addAnnotation (const std::string&);
  void removeAnnotations ();

  void addDependency (int);
  void removeDependency (int);
  void removeDependency (const std::string&);
  void getDependencies (std::vector <int>&) const;
  void getDependencies (std::vector <std::string>&) const;

  void substitute (const std::string&, const std::string&, bool);

  void validate () const;

  float urgency_c () const;
  float urgency ();

private:
  int determineVersion (const std::string&);
  void legacyParse (const std::string&);

  inline float urgency_priority () const;
  inline float urgency_project () const;
  inline float urgency_active () const;
  inline float urgency_waiting () const;
  inline float urgency_blocked () const;
  inline float urgency_annotations () const;
  inline float urgency_tags () const;
  inline float urgency_next () const;
  inline float urgency_due () const;
  inline float urgency_blocking () const;
};

#endif
////////////////////////////////////////////////////////////////////////////////
