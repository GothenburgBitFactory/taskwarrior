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

#ifndef INCLUDED_RECUR
#define INCLUDED_RECUR

#include <cmake.h>
// cmake.h include header must come first

#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Lexer.h>
#include <format.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unicode.h>
#include <unistd.h>
#include <util.h>

#include <optional>

std::optional<Datetime> checked_add_datetime(Datetime& base, time_t delta);
void handleRecurrence();
bool generateDueDates(Task& parent, std::vector<Datetime>& allDue);
std::optional<Datetime> getNextRecurrence(Datetime& current, std::string& period);
void updateRecurrenceMask(Task& task);
void handleUntil();

#endif

////////////////////////////////////////////////////////////////////////////////
