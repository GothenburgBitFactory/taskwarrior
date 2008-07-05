////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Date.h"
#include "Table.h"
#include "task.h"
#include "../auto.h"

////////////////////////////////////////////////////////////////////////////////
bool confirm (const std::string& question)
{
  std::cout << question << " (y/n) ";
  std::string answer;
  std::cin >> answer;

  answer = trim (answer);
  if (answer == "y" || answer == "Y")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void delay (float f)
{
  struct timeval t;
  t.tv_sec = (int) f;
  t.tv_usec = int ((f - (int)f) * 1000000);

  select (0, NULL, NULL, NULL, &t);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
// Long version:
//   0-59        S seconds
//   60-3599     M minutes, S seconds
//   3600-86399  H hours, M minutes, S seconds
//   86400-      D days, H hours, M minutes, S seconds
// Short version:
//   0-59        S seconds
//   60-3599     M minutes, S seconds
//   3600-86399  H hours, M minutes, S seconds
//
void formatTimeDeltaDays (std::string& output, time_t delta)
{
  char formatted[24];
  float days = (float) delta / 86400.0;

  if (days > 365)
    sprintf (formatted, "%.1f yrs", (days / 365.2422));
  else if (days > 84)
    sprintf (formatted, "%1d mth%s", (int) (days / 30.6), ((int) (days / 30.6) == 1 ? "" : "s"));
  else if (days > 13)
    sprintf (formatted, "%d wk%s", (int) (days / 7.0), ((int) (days / 7.0) == 1 ? "" : "s"));
  else if (days > 5.0)
    sprintf (formatted, "%d day%s", (int) days, ((int) days == 1 ? "" : "s"));
  else if (days > 1.0)
    sprintf (formatted, "%.1f days", days);
  else if (days * 24 > 1.0)
    sprintf (formatted, "%d hr%s", (int) (days * 24.0), ((int) (days * 24.0) == 1 ? "" : "s"));
  else if (days * 24 * 60 > 1)
    sprintf (formatted, "%d min%s", (int) (days * 24 * 60), ((int) (days * 24 * 60) == 1 ? "" : "s"));
  else if (days * 24 * 60 * 60 > 1)
    sprintf (formatted, "%d sec%s", (int) (days * 24 * 60 * 60), ((int) (days * 24 * 60 * 60) == 1 ? "" : "s"));
  else
    strcpy (formatted, "-");

  output = formatted;
}

////////////////////////////////////////////////////////////////////////////////
std::string formatSeconds (time_t delta)
{
  char formatted[24];
  float days = (float) delta / 86400.0;

  if (days > 365)
    sprintf (formatted, "%.1f yrs", (days / 365.2422));
  else if (days > 84)
    sprintf (formatted, "%1d mth%s", (int) (days / 30.6), ((int) (days / 30.6) == 1 ? "" : "s"));
  else if (days > 13)
    sprintf (formatted, "%d wk%s", (int) (days / 7.0), ((int) (days / 7.0) == 1 ? "" : "s"));
  else if (days > 5.0)
    sprintf (formatted, "%d day%s", (int) days, ((int) days == 1 ? "" : "s"));
  else if (days > 1.0)
    sprintf (formatted, "%.1f days", days);
  else if (days * 24 > 1.0)
    sprintf (formatted, "%d hr%s", (int) (days * 24.0), ((int) (days * 24) == 1 ? "" : "s"));
  else if (days * 24 * 60 > 1)
    sprintf (formatted, "%d min%s", (int) (days * 24 * 60), ((int) (days * 24 * 60) == 1 ? "" : "s"));
  else if (days * 24 * 60 * 60 > 1)
    sprintf (formatted, "%d sec%s", (int) (days * 24 * 60 * 60), ((int) (days * 24 * 60 * 60) == 1 ? "" : "s"));
  else
    strcpy (formatted, "-");

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
int autoComplete (
  const std::string& partial,
  const std::vector<std::string>& list,
  std::vector<std::string>& matches)
{
  matches.erase (matches.begin (), matches.end ());

  // Handle trivial case. 
  unsigned int length = partial.length ();
  if (length)
  {
    for (unsigned int i = 0; i < list.size (); ++i)
    {
      // Special case where there is an exact match.
      if (partial == list[i])
      {
        matches.erase (matches.begin (), matches.end ());
        matches.push_back (list[i]);
        return 1;
      }

      // Maintain a list of partial matches.
      if (length <= list[i].length () &&
          ! strncmp (partial.c_str (), list[i].c_str (), length))
        matches.push_back (list[i]);
    }
  }

  return matches.size ();
}

////////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_UUID

#include <uuid/uuid.h>

const std::string uuid ()
{
  uuid_t id;
  uuid_generate (id);
  char buffer[100];
  uuid_unparse_lower (id, buffer);

  return std::string (buffer);
}

////////////////////////////////////////////////////////////////////////////////
#else
#warning "Using custom UUID generator"

#include <stdlib.h>
static char randomHexDigit ()
{
  static char digits[] = "0123456789abcdef";
#ifdef HAVE_RANDOM
  return digits[random () % 16];
#else
  return digits[rand () % 16];
#endif
}

////////////////////////////////////////////////////////////////////////////////
const std::string uuid ()
{
  // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  char id [48] = {0};
  id[0]  = randomHexDigit ();
  id[1]  = randomHexDigit ();
  id[2]  = randomHexDigit ();
  id[3]  = randomHexDigit ();
  id[4]  = randomHexDigit ();
  id[5]  = randomHexDigit ();
  id[6]  = randomHexDigit ();
  id[7]  = randomHexDigit ();
  id[8]  = '-';
  id[9]  = randomHexDigit ();
  id[10] = randomHexDigit ();
  id[11] = randomHexDigit ();
  id[12] = randomHexDigit ();
  id[13] = '-';
  id[14] = randomHexDigit ();
  id[15] = randomHexDigit ();
  id[16] = randomHexDigit ();
  id[17] = randomHexDigit ();
  id[18] = '-';
  id[19] = randomHexDigit ();
  id[20] = randomHexDigit ();
  id[21] = randomHexDigit ();
  id[22] = randomHexDigit ();
  id[23] = '-';
  id[24] = randomHexDigit ();
  id[25] = randomHexDigit ();
  id[26] = randomHexDigit ();
  id[27] = randomHexDigit ();
  id[28] = randomHexDigit ();
  id[29] = randomHexDigit ();
  id[30] = randomHexDigit ();
  id[31] = randomHexDigit ();
  id[32] = randomHexDigit ();
  id[33] = randomHexDigit ();
  id[34] = randomHexDigit ();
  id[35] = randomHexDigit ();

  return id;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Recognize the following constructs, and return the number of days represented
int convertDuration (const std::string& input)
{
  std::string in (lowerCase (input));
  Date today;

  std::vector <std::string> supported;
  supported.push_back ("daily");
  supported.push_back ("day");
  supported.push_back ("weekly");
  supported.push_back ("sennight");
  supported.push_back ("biweekly");
  supported.push_back ("fortnight");
  supported.push_back ("monthly");
  supported.push_back ("bimonthly");
  supported.push_back ("semimonthly");
  supported.push_back ("quarterly");
  supported.push_back ("biannual");
  supported.push_back ("biyearly");
  supported.push_back ("annual");
  supported.push_back ("semiannual");
  supported.push_back ("yearly");

  std::vector <std::string> matches;
  if (autoComplete (in, supported, matches) == 1)
  {
    std::string found = matches[0];

         if (found == "daily"    || found == "day")       return 1;
    else if (found == "weekly"   || found == "sennight")  return 7;
    else if (found == "biweekly" || found == "fortnight") return 14;
    else if (found == "semimonthly")                      return 15;
    else if (found == "monthly")                          return 30;
    else if (found == "bimonthly")                        return 61;
    else if (found == "quarterly")                        return 91;
    else if (found == "semiannual")                       return 183;
    else if (found == "yearly"   || found == "annual")    return 365;
    else if (found == "biannual" || found == "biyearly")  return 730;
  }

  // Support \d+ d|w|m|q|y

  else
  {
    // Verify all digits followed by d, w, m, q, or y.
    int length = input.length ();
    for (unsigned int i = 0; i < length; ++i)
    {
      if (! isdigit (input[i]) &&
          i == length - 1)
      {
        int number = ::atoi (input.substr (0, i).c_str ());

        switch (input[length - 1])
        {
        case 'd': return number *   1; break;
        case 'w': return number *   7; break;
        case 'm': return number *  30; break;
        case 'q': return number *  91; break;
        case 'y': return number * 365; break;
        }
      }
    }
  }

  return 0; // Error.
}

////////////////////////////////////////////////////////////////////////////////
int addDuration (const Date& base, const std::string& offset)
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
