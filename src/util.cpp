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
    sprintf (formatted, "%1d mths", (int) (days / 30.6));
  else if (days > 13)
    sprintf (formatted, "%d wks", (int) (days / 7.0));
  else if (days > 5.0)
    sprintf (formatted, "%d days", (int) days);
  else if (days > 1.0)
    sprintf (formatted, "%.1f days", days);
  else if (days * 24 > 1.0)
    sprintf (formatted, "%d hrs", (int) (days * 24.0));
  else if (days * 24 * 60 > 1)
    sprintf (formatted, "%d mins", (int) (days * 24 * 60));
  else if (days * 24 * 60 * 60 > 1)
    sprintf (formatted, "%d secs", (int) (days * 24 * 60 * 60));
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
    sprintf (formatted, "%1d mths", (int) (days / 30.6));
  else if (days > 13)
    sprintf (formatted, "%d wks", (int) (days / 7.0));
  else if (days > 5.0)
    sprintf (formatted, "%d days", (int) days);
  else if (days > 1.0)
    sprintf (formatted, "%.1f days", days);
  else if (days * 24 > 1.0)
    sprintf (formatted, "%d hrs", (int) (days * 24.0));
  else if (days * 24 * 60 > 1)
    sprintf (formatted, "%d mins", (int) (days * 24 * 60));
  else if (days * 24 * 60 * 60 > 1)
    sprintf (formatted, "%d secs", (int) (days * 24 * 60 * 60));
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
  return digits[random () % 16];
}

////////////////////////////////////////////////////////////////////////////////
const std::string uuid ()
{
  // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  char id [37];
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
