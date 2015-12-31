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

#ifndef INCLUDED_MAIN
#define INCLUDED_MAIN

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <Context.h>
#include <ISO8601.h>
#include <Color.h>

// recur.cpp
void handleRecurrence ();
ISO8601d getNextRecurrence (ISO8601d&, std::string&);
bool generateDueDates (Task&, std::vector <ISO8601d>&);
void updateRecurrenceMask (Task&);
bool nag (Task&);

// rules.cpp
void initializeColorRules ();
void autoColorize (Task&, Color&);
std::string colorizeHeader (const std::string&);
std::string colorizeFootnote (const std::string&);
std::string colorizeError (const std::string&);
std::string colorizeDebug (const std::string&);

// dependency.cpp
void dependencyGetBlocked (const Task&, std::vector <Task>&);
void dependencyGetBlocking (const Task&, std::vector <Task>&);
bool dependencyIsCircular (const Task&);
void dependencyChainOnComplete (Task&);
void dependencyChainOnStart (Task&);

// feedback.cpp
std::string taskDifferences (const Task&, const Task&);
std::string taskInfoDifferences (const Task&, const Task&, const std::string&, long&, const long);
std::string renderAttribute (const std::string&, const std::string&, const std::string& format = "");
void feedback_affected (const std::string&);
void feedback_affected (const std::string&, int);
void feedback_affected (const std::string&, const Task&);
void feedback_reserved_tags (const std::string&);
void feedback_special_tags (const Task&, const std::string&);
void feedback_unblocked (const Task&);
void feedback_backlog ();
std::string onProjectChange (Task&, bool scope = true);
std::string onProjectChange (Task&, Task&);
std::string onExpiration (Task&);

// sort.cpp
void sort_tasks (std::vector <Task>&, std::vector <int>&, const std::string&);

// legacy.cpp
void legacyColumnMap (std::string&);
void legacySortColumnMap (std::string&);
std::string legacyCheckForDeprecatedVariables ();
std::string legacyCheckForDeprecatedColumns ();
void legacyAttributeMap (std::string&);

// list template
///////////////////////////////////////////////////////////////////////////////
template <class T> void listDiff (
  const T& left, const T& right, T& leftOnly, T& rightOnly)
{
  leftOnly.clear ();
  for (auto& l : left)
    if (std::find (right.begin (), right.end (), l) == right.end ())
      leftOnly.push_back (l);

  rightOnly.clear ();
  for (auto& r : right)
    if (std::find (left.begin (), left.end (), r) == left.end ())
      rightOnly.push_back (r);
}

#endif
////////////////////////////////////////////////////////////////////////////////
