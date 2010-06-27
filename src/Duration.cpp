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

  // Support \d+ \s? s|secs?|m|mins?|h|hrs?|d|days?|wks?|mo|mths?|y|yrs?|-
  // Note: Does not support a sign character.  That must be external to
  // Duration.
  Nibbler n (lower_input);
  int value;
  if (n.getUnsignedInt (value))
  {
    n.skip (' ');

         if (n.getLiteral ("yrs")  && n.depleted ()) return true;
    else if (n.getLiteral ("yr")   && n.depleted ()) return true;
    else if (n.getLiteral ("y")    && n.depleted ()) return true;

    else if (n.getLiteral ("qtrs") && n.depleted ()) return true;
    else if (n.getLiteral ("qtr")  && n.depleted ()) return true;
    else if (n.getLiteral ("q")    && n.depleted ()) return true;

    else if (n.getLiteral ("mths") && n.depleted ()) return true;
    else if (n.getLiteral ("mth")  && n.depleted ()) return true;
    else if (n.getLiteral ("mo")   && n.depleted ()) return true;

    else if (n.getLiteral ("wks")  && n.depleted ()) return true;
    else if (n.getLiteral ("wk")   && n.depleted ()) return true;
    else if (n.getLiteral ("w")    && n.depleted ()) return true;

    else if (n.getLiteral ("days") && n.depleted ()) return true;
    else if (n.getLiteral ("day")  && n.depleted ()) return true;
    else if (n.getLiteral ("d")    && n.depleted ()) return true;

    else if (n.getLiteral ("hrs")  && n.depleted ()) return true;
    else if (n.getLiteral ("hr")   && n.depleted ()) return true;
    else if (n.getLiteral ("h")    && n.depleted ()) return true;

    else if (n.getLiteral ("mins") && n.depleted ()) return true;
    else if (n.getLiteral ("min")  && n.depleted ()) return true;
    else if (n.getLiteral ("m")    && n.depleted ()) return true;

    else if (n.getLiteral ("secs") && n.depleted ()) return true;
    else if (n.getLiteral ("sec")  && n.depleted ()) return true;
    else if (n.getLiteral ("s")    && n.depleted ()) return true;

    else if (n.getLiteral ("-")    && n.depleted ()) return true;
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

  // Support \d+ \s? s|secs?|m|mins?|h|hrs?|d|days?|wks?|mo|mths?|y|yrs?|-
  // Note: Does not support a sign character.  That must be external to
  // Duration.
  else
  {
    Nibbler n (lower_input);
    int value;
    if (n.getUnsignedInt (value))
    {
      n.skip (' ');

           if (n.getLiteral ("yrs")  && n.depleted ()) mDays = value * 365;
      else if (n.getLiteral ("yr")   && n.depleted ()) mDays = value * 365;
      else if (n.getLiteral ("y")    && n.depleted ()) mDays = value * 365;

      else if (n.getLiteral ("qtrs") && n.depleted ()) mDays = value * 91;
      else if (n.getLiteral ("qtr")  && n.depleted ()) mDays = value * 91;
      else if (n.getLiteral ("q")    && n.depleted ()) mDays = value * 91;

      else if (n.getLiteral ("mths") && n.depleted ()) mDays = value * 30;
      else if (n.getLiteral ("mth")  && n.depleted ()) mDays = value * 30;
      else if (n.getLiteral ("mo")   && n.depleted ()) mDays = value * 30;

      else if (n.getLiteral ("wks")  && n.depleted ()) mDays = value * 7;
      else if (n.getLiteral ("wk")   && n.depleted ()) mDays = value * 7;
      else if (n.getLiteral ("w")    && n.depleted ()) mDays = value * 7;

      else if (n.getLiteral ("days") && n.depleted ()) mDays = value * 1;
      else if (n.getLiteral ("day")  && n.depleted ()) mDays = value * 1;
      else if (n.getLiteral ("d")    && n.depleted ()) mDays = value * 1;

      else if (n.getLiteral ("hrs")  && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("hr")   && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("h")    && n.depleted ()) mDays = 0;

      else if (n.getLiteral ("mins") && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("min")  && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("m")    && n.depleted ()) mDays = 0;

      else if (n.getLiteral ("secs") && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("sec")  && n.depleted ()) mDays = 0;
      else if (n.getLiteral ("s")    && n.depleted ()) mDays = 0;

      else if (n.getLiteral ("-")    && n.depleted ()) mDays = 0;
    }
  }

  if (mDays == 0)
    throw std::string ("The duration '") + input + "' was not recognized.";  // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
