////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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

#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "text.h"
#include "util.h"
#include "Duration.h"

////////////////////////////////////////////////////////////////////////////////
Duration::Duration ()
: mDays (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (time_t input)
{
  mDays = input;
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator time_t ()
{
  return mDays;
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator std::string ()
{
  std::stringstream s;
  s << mDays;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator< (const Duration& other)
{
  return mDays < other.mDays;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator> (const Duration& other)
{
  return mDays > other.mDays;
}

////////////////////////////////////////////////////////////////////////////////
Duration::~Duration ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::valid (const std::string& input) const
{
  std::string lower_input = lowerCase (input);

  std::vector <std::string> supported;
  supported.push_back ("daily");       // TODO i18n
  supported.push_back ("day");         // TODO i18n
  supported.push_back ("weekly");      // TODO i18n
  supported.push_back ("weekdays");    // TODO i18n
  supported.push_back ("sennight");    // TODO i18n
  supported.push_back ("biweekly");    // TODO i18n
  supported.push_back ("fortnight");   // TODO i18n
  supported.push_back ("monthly");     // TODO i18n
  supported.push_back ("bimonthly");   // TODO i18n
  supported.push_back ("quarterly");   // TODO i18n
  supported.push_back ("biannual");    // TODO i18n
  supported.push_back ("biyearly");    // TODO i18n
  supported.push_back ("annual");      // TODO i18n
  supported.push_back ("semiannual");  // TODO i18n
  supported.push_back ("yearly");      // TODO i18n

  std::vector <std::string> matches;
  if (autoComplete (lower_input, supported, matches) == 1)
    return true;

  // Support \d+ d|w|m|q|y
  // Verify all digits followed by d, w, m, q, or y.
  unsigned int length = lower_input.length ();
  for (unsigned int i = 0; i < length; ++i)
  {
    if (! isdigit (lower_input[i]) &&
        i == length - 1)
    {
      std::string type = lower_input.substr (length - 1);
      if (type == "d" ||   // TODO i18n
          type == "w" ||   // TODO i18n
          type == "m" ||   // TODO i18n
          type == "q" ||   // TODO i18n
          type == "y")     // TODO i18n
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Duration::parse (const std::string& input)
{
  std::string lower_input = lowerCase (input);

  std::vector <std::string> supported;
  supported.push_back ("daily");        // TODO i18n
  supported.push_back ("day");          // TODO i18n
  supported.push_back ("weekly");       // TODO i18n
  supported.push_back ("weekdays");     // TODO i18n
  supported.push_back ("sennight");     // TODO i18n
  supported.push_back ("biweekly");     // TODO i18n
  supported.push_back ("fortnight");    // TODO i18n
  supported.push_back ("monthly");      // TODO i18n
  supported.push_back ("bimonthly");    // TODO i18n
  supported.push_back ("quarterly");    // TODO i18n
  supported.push_back ("biannual");     // TODO i18n
  supported.push_back ("biyearly");     // TODO i18n
  supported.push_back ("annual");       // TODO i18n
  supported.push_back ("semiannual");   // TODO i18n
  supported.push_back ("yearly");       // TODO i18n

  std::vector <std::string> matches;
  if (autoComplete (lower_input, supported, matches) == 1)
  {
    std::string found = matches[0];

         if (found == "daily"    || found == "day")       mDays = 1;      // TODO i18n
    else if (found == "weekdays")                         mDays = 1;      // TODO i18n
    else if (found == "weekly"   || found == "sennight")  mDays = 7;      // TODO i18n
    else if (found == "biweekly" || found == "fortnight") mDays = 14;     // TODO i18n
    else if (found == "monthly")                          mDays = 30;     // TODO i18n
    else if (found == "bimonthly")                        mDays = 61;     // TODO i18n
    else if (found == "quarterly")                        mDays = 91;     // TODO i18n
    else if (found == "semiannual")                       mDays = 183;    // TODO i18n
    else if (found == "yearly"   || found == "annual")    mDays = 365;    // TODO i18n
    else if (found == "biannual" || found == "biyearly")  mDays = 730;    // TODO i18n
  }

  // Support \d+ d|w|m|q|y
  else
  {
    // Verify all digits followed by d, w, m, q, or y.
    unsigned int length = lower_input.length ();
    for (unsigned int i = 0; i < length; ++i)
    {
      if (! isdigit (lower_input[i]) &&
          i == length - 1)
      {
        int number = atoi (lower_input.substr (0, i).c_str ());

        switch (lower_input[length - 1])
        {
        case 'd': mDays = number *   1; break;   // TODO i18n
        case 'w': mDays = number *   7; break;   // TODO i18n
        case 'm': mDays = number *  30; break;   // TODO i18n
        case 'q': mDays = number *  91; break;   // TODO i18n
        case 'y': mDays = number * 365; break;   // TODO i18n
        }
      }
    }
  }

  if (mDays == 0)
    throw std::string ("The duration '") + input + "' was not recognized.";  // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
