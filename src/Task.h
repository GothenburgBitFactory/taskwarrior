////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_TASK
#define INCLUDED_TASK
#define L10N                                           // Localization complete.

#include <vector>
#include <map>
#include <string>
#include <stdio.h>

class Task : public std::map <std::string, std::string>
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

  bool has (const std::string&) const;
  std::vector <std::string> all ();
  const std::string get (const std::string&) const;
  int get_int (const std::string&) const;
  unsigned long get_ulong (const std::string&) const;
  time_t get_date (const std::string&) const;
  time_t get_duration (const std::string&) const;
  void set (const std::string&, const std::string&);
  void set (const std::string&, int);
  void remove (const std::string&);

  status getStatus () const;
  void setStatus (status);

  int getTagCount () const;
  bool hasTag (const std::string&) const;
  void addTag (const std::string&);
  void addTags (const std::vector <std::string>&);
  void getTags (std::vector<std::string>&) const;
  void removeTag (const std::string&);

  void getAnnotations (std::map <std::string, std::string>&) const;
  void setAnnotations (const std::map <std::string, std::string>&);
  void addAnnotation (const std::string&);
  void removeAnnotations ();

  void addDependency (int);
  void removeDependency (int);
  void removeDependency (const std::string&);
  void getDependencies (std::vector <int>&) const;
  void getDependencies (std::vector <std::string>&) const;

  void substitute (const std::string&, const std::string&, bool);

  void validate ();

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
  inline float urgency_age (float, float) const;
};

#endif
////////////////////////////////////////////////////////////////////////////////
