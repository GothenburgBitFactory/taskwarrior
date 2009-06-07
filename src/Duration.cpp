////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
Duration::Duration (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator int ()
{
  return (int) mDays;
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator time_t ()
{
  return mDays;
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
void Duration::parse (const std::string& input)
{
  std::string lower_input = lowerCase (input);

  std::vector <std::string> supported;
  supported.push_back ("daily");
  supported.push_back ("day");
  supported.push_back ("weekly");
  supported.push_back ("weekdays");
  supported.push_back ("sennight");
  supported.push_back ("biweekly");
  supported.push_back ("fortnight");
  supported.push_back ("monthly");
  supported.push_back ("bimonthly");
  supported.push_back ("quarterly");
  supported.push_back ("biannual");
  supported.push_back ("biyearly");
  supported.push_back ("annual");
  supported.push_back ("semiannual");
  supported.push_back ("yearly");

  std::vector <std::string> matches;
  if (autoComplete (lower_input, supported, matches) == 1)
  {
    std::string found = matches[0];

         if (found == "daily"    || found == "day")       mDays = 1;
    else if (found == "weekdays")                         mDays = 1;
    else if (found == "weekly"   || found == "sennight")  mDays = 7;
    else if (found == "biweekly" || found == "fortnight") mDays = 14;
    else if (found == "monthly")                          mDays = 30;
    else if (found == "bimonthly")                        mDays = 61;
    else if (found == "quarterly")                        mDays = 91;
    else if (found == "semiannual")                       mDays = 183;
    else if (found == "yearly"   || found == "annual")    mDays = 365;
    else if (found == "biannual" || found == "biyearly")  mDays = 730;
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
        int number = ::atoi (lower_input.substr (0, i).c_str ());

        switch (lower_input[length - 1])
        {
        case 'd': mDays = number *   1; break;
        case 'w': mDays = number *   7; break;
        case 'm': mDays = number *  30; break;
        case 'q': mDays = number *  91; break;
        case 'y': mDays = number * 365; break;
        }
      }
    }
  }

  if (mDays == 0)
    throw std::string ("The duration '") + input + "' was not recognized.";
}

////////////////////////////////////////////////////////////////////////////////
