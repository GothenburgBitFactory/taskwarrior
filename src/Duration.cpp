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
: mSecs (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (time_t input)
{
  mSecs = input;
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator time_t ()
{
  return mSecs;
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator std::string ()
{
  std::stringstream s;
  s << mSecs;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
Duration& Duration::operator= (const Duration& other)
{
  if (this != &other)
    mSecs = other.mSecs;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::format () const
{
  char formatted[24];
  float days = (float) mSecs / 86400.0;

  if (mSecs >= 86400 * 365)
    sprintf (formatted, "%.1f yrs", (days / 365));
  else if (mSecs > 86400 * 84)
    sprintf (formatted, "%1d mth%s",
                         (int) (float) (days / 30.6),
                        ((int) (float) (days / 30.6) == 1 ? "" : "s"));
  else if (mSecs > 86400 * 13)
    sprintf (formatted, "%d wk%s",
                         (int) (float) (days / 7.0),
                        ((int) (float) (days / 7.0) == 1 ? "" : "s"));
  else if (mSecs >= 86400)
    sprintf (formatted, "%d day%s",
                         (int) days,
                        ((int) days == 1 ? "" : "s"));
  else if (mSecs >= 3600)
    sprintf (formatted, "%d hr%s",
                         (int) (float) (mSecs / 3600),
                        ((int) (float) (mSecs / 3600) == 1 ? "" : "s"));
  else if (mSecs >= 60)
    sprintf (formatted, "%d min%s",
                         (int) (float) (mSecs / 60),
                        ((int) (float) (mSecs / 60) == 1 ? "" : "s"));
  else if (mSecs >= 1)
    sprintf (formatted, "%d sec%s",
                         (int) mSecs,
                        ((int) mSecs == 1 ? "" : "s"));
  else
    strcpy (formatted, "-"); // no i18n

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::formatCompact () const
{
  char formatted[24];
  float days = (float) mSecs / 86400.0;

       if (mSecs >= 86400 * 365) sprintf (formatted, "%.1fy", (days / 365));
  else if (mSecs >= 86400 * 84)  sprintf (formatted, "%1dmo", (int) (float) (days / 30.6));
  else if (mSecs >= 86400 * 13)  sprintf (formatted, "%dwk",  (int) (float) (days / 7.0));
  else if (mSecs >= 86400)       sprintf (formatted, "%dd",   (int) days);
  else if (mSecs >= 3600)        sprintf (formatted, "%dh",   (int) (float) (mSecs / 3600));
  else if (mSecs >= 60)          sprintf (formatted, "%dm",   (int) (float) (mSecs / 60));
  else if (mSecs >= 1)           sprintf (formatted, "%ds",   (int) mSecs);
  else                           strcpy (formatted, "-");

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator< (const Duration& other)
{
  return mSecs < other.mSecs;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator> (const Duration& other)
{
  return mSecs > other.mSecs;
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

         if (found == "daily"    || found == "day")       mSecs = 86400 * 1;   // TODO i18n
    else if (found == "weekdays")                         mSecs = 86400 * 1;   // TODO i18n
    else if (found == "weekly"   || found == "sennight")  mSecs = 86400 * 7;   // TODO i18n
    else if (found == "biweekly" || found == "fortnight") mSecs = 86400 * 14;  // TODO i18n
    else if (found == "monthly")                          mSecs = 86400 * 30;  // TODO i18n
    else if (found == "bimonthly")                        mSecs = 86400 * 61;  // TODO i18n
    else if (found == "quarterly")                        mSecs = 86400 * 91;  // TODO i18n
    else if (found == "semiannual")                       mSecs = 86400 * 183; // TODO i18n
    else if (found == "yearly"   || found == "annual")    mSecs = 86400 * 365; // TODO i18n
    else if (found == "biannual" || found == "biyearly")  mSecs = 86400 * 730; // TODO i18n
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

           if (n.getLiteral ("yrs")  && n.depleted ()) mSecs = value * 86400 * 365;
      else if (n.getLiteral ("yr")   && n.depleted ()) mSecs = value * 86400 * 365;
      else if (n.getLiteral ("y")    && n.depleted ()) mSecs = value * 86400 * 365;

      else if (n.getLiteral ("qtrs") && n.depleted ()) mSecs = value * 86400 * 91;
      else if (n.getLiteral ("qtr")  && n.depleted ()) mSecs = value * 86400 * 91;
      else if (n.getLiteral ("q")    && n.depleted ()) mSecs = value * 86400 * 91;

      else if (n.getLiteral ("mths") && n.depleted ()) mSecs = value * 86400 * 30;
      else if (n.getLiteral ("mth")  && n.depleted ()) mSecs = value * 86400 * 30;
      else if (n.getLiteral ("mo")   && n.depleted ()) mSecs = value * 86400 * 30;

      else if (n.getLiteral ("wks")  && n.depleted ()) mSecs = value * 86400 * 7;
      else if (n.getLiteral ("wk")   && n.depleted ()) mSecs = value * 86400 * 7;
      else if (n.getLiteral ("w")    && n.depleted ()) mSecs = value * 86400 * 7;

      else if (n.getLiteral ("days") && n.depleted ()) mSecs = value * 86400;
      else if (n.getLiteral ("day")  && n.depleted ()) mSecs = value * 86400;
      else if (n.getLiteral ("d")    && n.depleted ()) mSecs = value * 86400;

      else if (n.getLiteral ("hrs")  && n.depleted ()) mSecs = value * 3600;
      else if (n.getLiteral ("hr")   && n.depleted ()) mSecs = value * 3600;
      else if (n.getLiteral ("h")    && n.depleted ()) mSecs = value * 3600;

      else if (n.getLiteral ("mins") && n.depleted ()) mSecs = value * 60;
      else if (n.getLiteral ("min")  && n.depleted ()) mSecs = value * 60;
      else if (n.getLiteral ("m")    && n.depleted ()) mSecs = value * 60;

      else if (n.getLiteral ("secs") && n.depleted ()) mSecs = value;
      else if (n.getLiteral ("sec")  && n.depleted ()) mSecs = value;
      else if (n.getLiteral ("s")    && n.depleted ()) mSecs = value;

      else if (n.getLiteral ("-")    && n.depleted ()) mSecs = 0;
    }
  }

  if (mSecs == 0)
    throw std::string ("The duration '") + input + "' was not recognized.";  // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
