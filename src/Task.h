////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <time.h>
#include <JSON.h>

class Task
{
public:
  static std::string defaultProject;
  static std::string defaultDue;
  static bool searchCaseSensitive;
  static bool regex;
  static std::map <std::string, std::string> attributes;  // name -> type
  static std::map <std::string, float> coefficients;
  static std::map <std::string, std::vector <std::string>> customOrder;
  static float urgencyProjectCoefficient;
  static float urgencyActiveCoefficient;
  static float urgencyScheduledCoefficient;
  static float urgencyWaitingCoefficient;
  static float urgencyBlockedCoefficient;
  static float urgencyAnnotationsCoefficient;
  static float urgencyTagsCoefficient;
  static float urgencyDueCoefficient;
  static float urgencyBlockingCoefficient;
  static float urgencyAgeCoefficient;
  static float urgencyAgeMax;

public:
  Task () = default;
  bool operator== (const Task&);
  Task (const std::string&);
  Task (const json::object*);

  void parse (const std::string&);
  std::string composeF4 () const;
  std::string composeJSON (bool decorate = false) const;

  // Status values.
  enum status {pending, completed, deleted, recurring, waiting};

  // Date state values.
  enum dateState {dateNotDue, dateAfterToday, dateLaterToday, dateEarlierToday, dateBeforeToday};

  // Public data.
  std::map <std::string, std::string> data {};
  int id                                   {0};
  float urgency_value                      {0.0};
  bool recalc_urgency                      {true};
  bool is_blocked                          {false};
  bool is_blocking                         {false};
  int annotation_count                     {0};

  // Series of helper functions.
  static status textToStatus (const std::string&);
  static std::string statusToText (status);

  void setAsNow (const std::string&);
  bool has (const std::string&) const;
  std::vector <std::string> all ();
  const std::string identifier (bool shortened = false) const;
  const std::string get (const std::string&) const;
  const std::string& get_ref (const std::string&) const;
  int get_int (const std::string&) const;
  unsigned long get_ulong (const std::string&) const;
  float get_float (const std::string&) const;
  time_t get_date (const std::string&) const;
  void set (const std::string&, const std::string&);
  void set (const std::string&, int);
  void remove (const std::string&);

#ifdef PRODUCT_TASKWARRIOR
  bool is_ready () const;
  bool is_due () const;
  bool is_dueyesterday () const;
  bool is_duetoday () const;
  bool is_duetomorrow () const;
  bool is_dueweek () const;
  bool is_duemonth () const;
  bool is_dueyear () const;
  bool is_overdue () const;
  bool is_udaPresent () const;
  bool is_orphanPresent () const;
#endif

  status getStatus () const;
  void setStatus (status);

#ifdef PRODUCT_TASKWARRIOR
  dateState getDateState (const std::string&) const;
#endif

  int getTagCount () const;
  bool hasTag (const std::string&) const;
  void addTag (const std::string&);
  void addTags (const std::vector <std::string>&);
  void getTags (std::vector<std::string>&) const;
  void removeTag (const std::string&);

  bool hasAnnotations () const;
  void getAnnotations (std::map <std::string, std::string>&) const;
  void setAnnotations (const std::map <std::string, std::string>&);
  void addAnnotation (const std::string&);
  void removeAnnotations ();

#ifdef PRODUCT_TASKWARRIOR
  void addDependency (int);
#endif
  void addDependency (const std::string&);
#ifdef PRODUCT_TASKWARRIOR
  void removeDependency (int);
  void removeDependency (const std::string&);
  void getDependencies (std::vector <int>&) const;
  void getDependencies (std::vector <std::string>&) const;

  void getUDAOrphans (std::vector <std::string>&) const;

  void substitute (const std::string&, const std::string&, const std::string&);
#endif

  void validate (bool applyDefault = true);

  float urgency_c () const;
  float urgency ();

#ifdef PRODUCT_TASKWARRIOR
  enum modType {modReplace, modPrepend, modAppend, modAnnotate};
  void modify (modType, bool text_required = false);
#endif

private:
  int determineVersion (const std::string&);
  void parseJSON (const std::string&);
  void parseJSON (const json::object*);
  void parseLegacy (const std::string&);
  void validate_before (const std::string&, const std::string&);
  const std::string encode (const std::string&) const;
  const std::string decode (const std::string&) const;

public:
  float urgency_project () const;
  float urgency_active () const;
  float urgency_scheduled () const;
  float urgency_waiting () const;
  float urgency_blocked () const;
  float urgency_inherit () const;
  float urgency_annotations () const;
  float urgency_tags () const;
  float urgency_due () const;
  float urgency_blocking () const;
  float urgency_age () const;
};

#endif
////////////////////////////////////////////////////////////////////////////////
