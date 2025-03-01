////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_MAIN
#define INCLUDED_MAIN

#include <Color.h>
#include <Context.h>
#include <Datetime.h>
#include <sys/types.h>

#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

// recur.cpp
void handleRecurrence();
void handleUntil();
std::optional<Datetime> checked_add_datetime(Datetime& base, time_t delta);
std::optional<Datetime> getNextRecurrence(Datetime&, std::string&);
bool generateDueDates(Task&, std::vector<Datetime>&);
void updateRecurrenceMask(Task&);

// nag.cpp
void nag(std::vector<Task>&);

// rules.cpp
void initializeColorRules();
void autoColorize(Task&, Color&);
std::string colorizeHeader(const std::string&);
std::string colorizeFootnote(const std::string&);
std::string colorizeError(const std::string&);
std::string colorizeDebug(const std::string&);

// dependency.cpp
bool dependencyIsCircular(const Task&);
void dependencyChainOnComplete(Task&);
void dependencyChainOnStart(Task&);

// feedback.cpp
std::string renderAttribute(const std::string&, const std::string&, const std::string& format = "");
void feedback_affected(const std::string&);
void feedback_affected(const std::string&, int);
void feedback_affected(const std::string&, const Task&);
void feedback_reserved_tags(const std::string&);
void feedback_special_tags(const Task&, const std::string&);
void feedback_unblocked(const Task&);
void feedback_backlog();
std::string onProjectChange(Task&, bool scope = true);
std::string onProjectChange(Task&, Task&);
std::string onExpiration(Task&);

// sort.cpp
void sort_tasks(std::vector<Task>&, std::vector<int>&, const std::string&);
void sort_projects(std::list<std::pair<std::string, int>>& sorted,
                   std::map<std::string, int>& allProjects);
void sort_projects(std::list<std::pair<std::string, int>>& sorted,
                   std::map<std::string, bool>& allProjects);

// legacy.cpp
void legacyColumnMap(std::string&);
void legacySortColumnMap(std::string&);
std::string legacyCheckForDeprecatedVariables();
std::string legacyCheckForDeprecatedColumns();
void legacyAttributeMap(std::string&);

#endif
////////////////////////////////////////////////////////////////////////////////
