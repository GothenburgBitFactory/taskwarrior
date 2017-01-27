////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_CMDHISTORY
#define INCLUDED_CMDHISTORY

#include <string>
#include <Command.h>
#include <Table.h>
#include <Datetime.h>
#include <i18n.h>

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

////////////////////////////////////////////////////////////////////////////i
class MonthlyHistoryStrategy
{
public:
  static Datetime getRelevantDate (const Datetime & dt)
  {
    return dt.startOfMonth ();
  }

  static void setupTableDates (Table & view)
  {
    view.add (STRING_CMD_HISTORY_YEAR);
    view.add (STRING_CMD_HISTORY_MONTH);
  }

  static void insertRowDate (
    Table& view,
    int row,
    time_t rowTime,
    time_t lastTime)
  {
    Datetime dt (rowTime);
    int m, d, y;
    dt.toYMD (y, m, d);

    Datetime last_dt (lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD (last_y, last_m, last_d);

    if (y != last_y)
    {
      view.set (row, 0, y);
    }

    view.set (row, 1, Datetime::monthName (m));
  }

  static constexpr const char* keyword         = "history.monthly";
  static constexpr const char* usage           = "task <filter> history.monthly";
  static constexpr const char* description     = STRING_CMD_HISTORY_USAGE_M;
  static constexpr unsigned int dateFieldCount = 2;
  static constexpr bool graphical              = false;
};

typedef CmdHistoryBase<MonthlyHistoryStrategy> CmdHistoryMonthly;


////////////////////////////////////////////////////////////////////////////i
class MonthlyGHistoryStrategy
{
public:
  static Datetime getRelevantDate (const Datetime & dt)
  {
    return dt.startOfMonth ();
  }

  static void setupTableDates (Table & view)
  {
    view.add (STRING_CMD_HISTORY_YEAR);
    view.add (STRING_CMD_HISTORY_MONTH);
  }

  static void insertRowDate (
    Table& view,
     int row,
     time_t rowTime,
     time_t lastTime)
  {
    Datetime dt (rowTime);
    int m, d, y;
    dt.toYMD (y, m, d);

    Datetime last_dt (lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD (last_y, last_m, last_d);

    if (y != last_y)
    {
      view.set (row, 0, y);
    }

    view.set (row, 1, Datetime::monthName (m));
  }

  static constexpr const char* keyword         = "ghistory.monthly";
  static constexpr const char* usage           = "task <filter> ghistory.monthly";
  static constexpr const char* description     = STRING_CMD_GHISTORY_USAGE_M;
  static constexpr unsigned int dateFieldCount = 2;
  static constexpr bool graphical              = true;
};

typedef CmdHistoryBase<MonthlyGHistoryStrategy> CmdGHistoryMonthly;


////////////////////////////////////////////////////////////////////////////i
class AnnualHistoryStrategy
{
public:
  static Datetime getRelevantDate (const Datetime & dt) 
  {
    return dt.startOfYear ();
  }

  static void setupTableDates (Table & view)
  {
    view.add (STRING_CMD_HISTORY_YEAR);
  }

  static void insertRowDate (
    Table& view,
     int row,
     time_t rowTime,
     time_t lastTime)
  {
    Datetime dt (rowTime);
    int m, d, y;
    dt.toYMD (y, m, d);

    Datetime last_dt (lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD (last_y, last_m, last_d);

    if (y != last_y)
    {
      view.set (row, 0, y);
    }
  }

  static constexpr const char* keyword         = "history.annual";
  static constexpr const char* usage           = "task <filter> history.annual";
  static constexpr const char* description     = STRING_CMD_HISTORY_USAGE_A;
  static constexpr unsigned int dateFieldCount = 1;
  static constexpr bool graphical              = false;
};

typedef CmdHistoryBase<AnnualHistoryStrategy> CmdHistoryAnnual;


////////////////////////////////////////////////////////////////////////////i
class AnnualGHistoryStrategy
{
public:
  static Datetime getRelevantDate (const Datetime & dt)
  {
    return dt.startOfYear ();
  }

  static void setupTableDates (Table & view)
  {
    view.add (STRING_CMD_HISTORY_YEAR);
  }

  static void insertRowDate (
    Table& view,
    int row,
    time_t rowTime,
    time_t lastTime)
  {
    Datetime dt (rowTime);
    int m, d, y;
    dt.toYMD (y, m, d);

    Datetime last_dt (lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD (last_y, last_m, last_d);

    if (y != last_y)
    {
      view.set (row, 0, y);
    }
  }

  static constexpr const char* keyword         = "ghistory.annual";
  static constexpr const char* usage           = "task <filter> ghistory.annual";
  static constexpr const char* description     = STRING_CMD_GHISTORY_USAGE_A;
  static constexpr unsigned int dateFieldCount = 1;
  static constexpr bool graphical              = true;
};

typedef CmdHistoryBase<AnnualGHistoryStrategy> CmdGHistoryAnnual;

#endif
