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

#include <cmake.h>
// cmake.h include header must come first

#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Lexer.h>
#include <format.h>
#include <main.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unicode.h>
#include <unistd.h>
#include <util.h>

#include <limits>
#include <optional>

// Add a `time_t` delta to a Datetime, checking for and returning nullopt on integer overflow.
std::optional<Datetime> checked_add_datetime(Datetime& base, time_t delta) {
  // Datetime::operator+ takes an integer delta, so check that range
  if (static_cast<time_t>(std::numeric_limits<int>::max()) < delta) {
    return std::nullopt;
  }

  // Check for time_t overflow in the Datetime.
  if (std::numeric_limits<time_t>::max() - base.toEpoch() < delta) {
    return std::nullopt;
  }
  return base + delta;
}

////////////////////////////////////////////////////////////////////////////////
// Scans all tasks, and for any recurring tasks, determines whether any new
// child tasks need to be generated to fill gaps.
void handleRecurrence() {
  // Recurrence can be disabled.
  // Note: This is currently a workaround for TD-44, TW-1520.
  if (!Context::getContext().config.getBoolean("recurrence")) return;

  auto tasks = Context::getContext().tdb2.pending_tasks();
  Datetime now;

  // Look at all tasks and find any recurring ones.
  for (auto& t : tasks) {
    if (t.getStatus() == Task::recurring) {
      // Generate a list of due dates for this recurring task, regardless of
      // the mask.
      std::vector<Datetime> due;
      if (!generateDueDates(t, due)) {
        // Determine the end date.
        t.setStatus(Task::deleted);
        Context::getContext().tdb2.modify(t);
        Context::getContext().footnote(onExpiration(t));
        continue;
      }

      // Get the mask from the parent task.
      auto mask = t.get("mask");

      // Iterate over the due dates, and check each against the mask.
      auto changed = false;
      unsigned int i = 0;
      for (auto& d : due) {
        if (mask.length() <= i) {
          changed = true;

          Task rec(t);                       // Clone the parent.
          rec.setStatus(Task::pending);      // Change the status.
          rec.set("uuid", uuid());           // New UUID.
          rec.set("parent", t.get("uuid"));  // Remember mom.
          rec.setAsNow("entry");             // New entry date.
          rec.set("due", format(d.toEpoch()));

          if (t.has("wait")) {
            Datetime old_wait(t.get_date("wait"));
            Datetime old_due(t.get_date("due"));
            Datetime due(d);
            auto wait = checked_add_datetime(due, old_wait - old_due);
            if (wait) {
              rec.set("wait", format(wait->toEpoch()));
            } else {
              rec.remove("wait");
            }
            rec.setStatus(Task::waiting);
            mask += 'W';
          } else {
            mask += '-';
            rec.setStatus(Task::pending);
          }

          rec.set("imask", i);
          rec.remove("mask");  // Remove the mask of the parent.

          // Add the new task to the DB.
          Context::getContext().tdb2.add(rec);
        }

        ++i;
      }

      // Only modify the parent if necessary.
      if (changed) {
        t.set("mask", mask);
        Context::getContext().tdb2.modify(t);

        if (Context::getContext().verbose("recur"))
          Context::getContext().footnote(
              format("Creating recurring task instance '{1}'", t.get("description")));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Determine a start date (due), an optional end date (until), and an increment
// period (recur).  Then generate a set of corresponding dates.
//
// Returns false if the parent recurring task is depleted.
bool generateDueDates(Task& parent, std::vector<Datetime>& allDue) {
  // Determine due date, recur period and until date.
  Datetime due(parent.get_date("due"));
  if (due._date == 0) return false;

  std::string recur = parent.get("recur");

  bool specificEnd = false;
  Datetime until;
  if (parent.get("until") != "") {
    until = Datetime(parent.get("until"));
    specificEnd = true;
  }

  auto recurrence_limit = Context::getContext().config.getInteger("recurrence.limit");
  int recurrence_counter = 0;
  Datetime now;
  Datetime i = due;
  while (1) {
    allDue.push_back(i);

    if (specificEnd && i > until) {
      // If i > until, it means there are no more tasks to generate, and if the
      // parent mask contains all + or X, then there never will be another task
      // to generate, and this parent task may be safely reaped.
      auto mask = parent.get("mask");
      if (mask.length() == allDue.size() && mask.find('-') == std::string::npos) return false;

      return true;
    }

    if (i > now) ++recurrence_counter;

    if (recurrence_counter >= recurrence_limit) return true;
    auto next = getNextRecurrence(i, recur);
    if (next) {
      i = *next;
    } else {
      return true;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// Determine the next recurrence of the given period.
///
/// If no such date can be calculated, such as with a very large period, returns
/// nullopt.
std::optional<Datetime> getNextRecurrence(Datetime& current, std::string& period) {
  auto m = current.month();
  auto d = current.day();
  auto y = current.year();
  auto ho = current.hour();
  auto mi = current.minute();
  auto se = current.second();

  // Some periods are difficult, because they can be vague.
  if (period == "monthly" || period == "P1M") {
    if (++m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period == "weekdays") {
    auto dow = current.dayOfWeek();
    int days;

    if (dow == 5)
      days = 3;
    else if (dow == 6)
      days = 2;
    else
      days = 1;

    return checked_add_datetime(current, days * 86400);
  }

  else if (unicodeLatinDigit(period[0]) && period[period.length() - 1] == 'm') {
    int increment = strtol(period.substr(0, period.length() - 1).c_str(), nullptr, 10);

    if (increment <= 0)
      throw format("Recurrence period '{1}' is equivalent to {2} and hence invalid.", period,
                   increment);

    m += increment;
    while (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period[0] == 'P' && Lexer::isAllDigits(period.substr(1, period.length() - 2)) &&
           period[period.length() - 1] == 'M') {
    int increment = strtol(period.substr(1, period.length() - 2).c_str(), nullptr, 10);

    if (increment <= 0)
      throw format("Recurrence period '{1}' is equivalent to {2} and hence invalid.", period,
                   increment);

    m += increment;
    while (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d);
  }

  else if (period == "quarterly" || period == "P3M") {
    m += 3;
    if (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (unicodeLatinDigit(period[0]) && period[period.length() - 1] == 'q') {
    int increment = strtol(period.substr(0, period.length() - 1).c_str(), nullptr, 10);

    if (increment <= 0) {
      Context::getContext().footnote(format(
          "Recurrence period '{1}' is equivalent to {2} and hence invalid.", period, increment));
      return std::nullopt;
    }

    m += 3 * increment;
    while (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period == "semiannual" || period == "P6M") {
    m += 6;
    if (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period == "bimonthly" || period == "P2M") {
    m += 2;
    if (m > 12) {
      m -= 12;
      ++y;
    }

    while (!Datetime::valid(y, m, d)) --d;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period == "biannual" || period == "biyearly" || period == "P2Y") {
    y += 2;

    return Datetime(y, m, d, ho, mi, se);
  }

  else if (period == "annual" || period == "yearly" || period == "P1Y") {
    y += 1;

    // If the due data just happens to be 2/29 in a leap year, then simply
    // incrementing y is going to create an invalid date.
    if (m == 2 && d == 29) d = 28;

    return Datetime(y, m, d, ho, mi, se);
  }

  // Add the period to current, and we're done.
  std::string::size_type idx = 0;
  Duration p;
  if (!p.parse(period, idx)) {
    Context::getContext().footnote(
        format("Warning: The recurrence value '{1}' is not valid.", period));
    return std::nullopt;
  }

  return checked_add_datetime(current, p.toTime_t());
}

////////////////////////////////////////////////////////////////////////////////
// When the status of a recurring child task changes, the parent task must
// update it's mask.
void updateRecurrenceMask(Task& task) {
  auto uuid = task.get("parent");
  Task parent;

  if (uuid != "" && Context::getContext().tdb2.get(uuid, parent)) {
    unsigned int index = strtol(task.get("imask").c_str(), nullptr, 10);
    auto mask = parent.get("mask");
    if (mask.length() > index) {
      mask[index] = (task.getStatus() == Task::pending)     ? '-'
                    : (task.getStatus() == Task::completed) ? '+'
                    : (task.getStatus() == Task::deleted)   ? 'X'
                    : (task.getStatus() == Task::waiting)   ? 'W'
                                                            : '?';
    } else {
      std::string mask;
      for (unsigned int i = 0; i < index; ++i) mask += "?";

      mask += (task.getStatus() == Task::pending)     ? '-'
              : (task.getStatus() == Task::completed) ? '+'
              : (task.getStatus() == Task::deleted)   ? 'X'
              : (task.getStatus() == Task::waiting)   ? 'W'
                                                      : '?';
    }

    parent.set("mask", mask);
    Context::getContext().tdb2.modify(parent);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Delete expired tasks.
void handleUntil() {
  Datetime now;
  auto tasks = Context::getContext().tdb2.pending_tasks();
  for (auto& t : tasks) {
    // TODO What about expiring template tasks?
    if (t.getStatus() == Task::pending && t.has("until")) {
      auto until = Datetime(t.get_date("until"));
      if (until < now) {
        Context::getContext().debug(format("handleUntil: recurrence expired until {1} < now {2}",
                                           until.toISOLocalExtended(), now.toISOLocalExtended()));
        t.setStatus(Task::deleted);
        Context::getContext().tdb2.modify(t);
        Context::getContext().footnote(onExpiration(t));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
