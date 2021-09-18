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

#ifndef INCLUDED_CMDHISTORY
#define INCLUDED_CMDHISTORY

#include <string>
#include <Command.h>
#include <Table.h>
#include <Datetime.h>

template<class HistoryStrategy>
class CmdHistoryBase : public Command
{
public:
  CmdHistoryBase ();
  int execute (std::string&);

private:
  std::map <time_t, int> groups;          // Represents any timeinterval with data
  std::map <time_t, int> addedGroup;      // Additions by timeinterval
  std::map <time_t, int> completedGroup;  // Completions by timeinterval
  std::map <time_t, int> deletedGroup;    // Deletions by timeinterval
  int rc;

  void outputTabular (std::string&);
  void outputGraphical (std::string&);
};

// Forward-declare strategies implemented in CmdHistory.cpp
class DailyHistoryStrategy;
class DailyGHistoryStrategy;
class WeeklyHistoryStrategy;
class WeeklyGHistoryStrategy;
class MonthlyHistoryStrategy;
class MonthlyGHistoryStrategy;
class AnnualHistoryStrategy;
class AnnualGHistoryStrategy;

// typedef the templates to nice names to be used outside this class
typedef CmdHistoryBase<DailyHistoryStrategy>    CmdHistoryDaily;
typedef CmdHistoryBase<DailyGHistoryStrategy>   CmdGHistoryDaily;
typedef CmdHistoryBase<WeeklyHistoryStrategy>   CmdHistoryWeekly;
typedef CmdHistoryBase<WeeklyGHistoryStrategy>  CmdGHistoryWeekly;
typedef CmdHistoryBase<MonthlyHistoryStrategy>  CmdHistoryMonthly;
typedef CmdHistoryBase<MonthlyGHistoryStrategy> CmdGHistoryMonthly;
typedef CmdHistoryBase<AnnualHistoryStrategy>   CmdHistoryAnnual;
typedef CmdHistoryBase<AnnualGHistoryStrategy>  CmdGHistoryAnnual;

#endif
