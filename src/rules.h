////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2025, Tomas Babej, Paul Beckingham, Federico Hernandez,
// 						  Tobias Predel.
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

#ifndef INCLUDED_RULES
#define INCLUDED_RULES

#include <cmake.h>
// cmake.h include header must come first

#include <Context.h>
#include <Datetime.h>
#include <shared.h>

static std::map<std::string, Color> gsColor;
static std::vector<std::string> gsPrecedence;
static Datetime now;

void initializeColorRules();
static void applyColor(const Color& base, Color& c, bool merge);
static void colorizeBlocked(Task& task, const Color& base, Color& c, bool merge);
static void colorizeBlocking(Task& task, const Color& base, Color& c, bool merge);
static void colorizeTagged(Task& task, const Color& base, Color& c, bool merge);
static void colorizeActive(Task& task, const Color& base, Color& c, bool merge);
static void colorizeScheduled(Task& task, const Color& base, Color& c, bool merge);
static void colorizeUntil(Task& task, const Color& base, Color& c, bool merge);
static void colorizeTag(Task& task, const std::string& rule, const Color& base, Color& c,
                        bool merge);
static void colorizeProject(Task& task, const std::string& rule, const Color& base, Color& c,
                            bool merge);
static void colorizeProjectNone(Task& task, const Color& base, Color& c, bool merge);
static void colorizeTagNone(Task& task, const Color& base, Color& c, bool merge);
static void colorizeKeyword(Task& task, const std::string& rule, const Color& base, Color& c,
                            bool merge);
static void colorizeUDA(Task& task, const std::string& rule, const Color& base, Color& c,
                        bool merge);
static void colorizeDue(Task& task, const Color& base, Color& c, bool merge);
static void colorizeDueToday(Task& task, const Color& base, Color& c, bool merge);
static void colorizeOverdue(Task& task, const Color& base, Color& c, bool merge);
static void colorizeRecurring(Task& task, const Color& base, Color& c, bool merge);
static void colorizeCompleted(Task& task, const Color& base, Color& c, bool merge);
static void colorizeDeleted(Task& task, const Color& base, Color& c, bool merge);
void autoColorize(Task& task, Color& c);
std::string colorizeHeader(const std::string& input);
std::string colorizeFootnote(const std::string& input);
std::string colorizeError(const std::string& input);
std::string colorizeDebug(const std::string& input);

#endif

////////////////////////////////////////////////////////////////////////////////
