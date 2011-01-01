////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <stdio.h>
#include <string.h>
#include "text.h"
#include "util.h"
#include "Duration.h"

static const char* durations[] =
{
  "annual",
  "biannual",
  "bimonthly",
  "biweekly",
  "biyearly",
  "daily",
  "days",
  "day",
  "d",
  "fortnight",
  "hours",
  "hrs",
  "h",
  "minutes",
  "mins",
  "min",
  "m",

  "mnths",
  "monthly",
  "months",
  "month",
  "mos",
  "mo",
  "mths",

  "quarterly",
  "quarters",
  "qrtrs",
  "qtrs",
  "q",
  "seconds",
  "secs",
  "sec",
  "s",
  "semiannual",
  "sennight",
  "weekdays",
  "weekly",
  "weeks",
  "wks",
  "w",
  "yearly",
  "years",
  "yrs",
  "y",
  "-",
};

#define NUM_DURATIONS   (sizeof (durations)   / sizeof (durations[0]))

////////////////////////////////////////////////////////////////////////////////
Duration::Duration ()
: mSecs (0)
, mNegative (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const Duration& other)
{
  mSecs     = other.mSecs;
  mNegative = other.mNegative;
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (time_t input)
{
  if (input < 0)
  {
    mSecs = -input;
    mNegative = true;
  }
  else
  {
    mSecs = input;
    mNegative = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const std::string& input)
: mSecs (0)
, mNegative (false)
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
  s << (mNegative ? - (long) mSecs : (long) mSecs);
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
Duration& Duration::operator= (const Duration& other)
{
  if (this != &other)
  {
    mSecs     = other.mSecs;
    mNegative = other.mNegative;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::format () const
{
  char formatted[24];
  float days = (float) mSecs / 86400.0;

  if (mSecs >= 86400 * 365)
    sprintf (formatted, "%s%.1f yrs", (mNegative ? "-" : ""), (days / 365));
  else if (mSecs > 86400 * 84)
    sprintf (formatted, "%s%1d mth%s",
                        (mNegative ? "-" : ""),  (int) (float) (days / 30.6),
                        ((int) (float) (days / 30.6) == 1 ? "" : "s"));
  else if (mSecs > 86400 * 13)
    sprintf (formatted, "%s%d wk%s",
                        (mNegative ? "-" : ""),  (int) (float) (days / 7.0),
                        ((int) (float) (days / 7.0) == 1 ? "" : "s"));
  else if (mSecs >= 86400)
    sprintf (formatted, "%s%d day%s",
                        (mNegative ? "-" : ""),  (int) days,
                        ((int) days == 1 ? "" : "s"));
  else if (mSecs >= 3600)
    sprintf (formatted, "%s%d hr%s",
                        (mNegative ? "-" : ""),  (int) (float) (mSecs / 3600),
                        ((int) (float) (mSecs / 3600) == 1 ? "" : "s"));
  else if (mSecs >= 60)
    sprintf (formatted, "%s%d min%s",
                        (mNegative ? "-" : ""),  (int) (float) (mSecs / 60),
                        ((int) (float) (mSecs / 60) == 1 ? "" : "s"));
  else if (mSecs >= 1)
    sprintf (formatted, "%s%d sec%s",
                        (mNegative ? "-" : ""),  (int) mSecs,
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

       if (mSecs >= 86400 * 365) sprintf (formatted, "%s%.1fy", (mNegative ? "-" : ""), (days / 365));
  else if (mSecs >= 86400 * 84)  sprintf (formatted, "%s%1dmo", (mNegative ? "-" : ""), (int) (float) (days / 30.6));
  else if (mSecs >= 86400 * 13)  sprintf (formatted, "%s%dwk",  (mNegative ? "-" : ""), (int) (float) (days / 7.0));
  else if (mSecs >= 86400)       sprintf (formatted, "%s%dd",   (mNegative ? "-" : ""), (int) days);
  else if (mSecs >= 3600)        sprintf (formatted, "%s%dh",   (mNegative ? "-" : ""), (int) (float) (mSecs / 3600));
  else if (mSecs >= 60)          sprintf (formatted, "%s%dm",   (mNegative ? "-" : ""), (int) (float) (mSecs / 60));
  else if (mSecs >= 1)           sprintf (formatted, "%s%ds",   (mNegative ? "-" : ""), (int) mSecs);
  else                           strcpy (formatted, "-");

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator< (const Duration& other)
{
  long left  = (long) (      mNegative ?       -mSecs :       mSecs);
  long right = (long) (other.mNegative ? -other.mSecs : other.mSecs);

  return left < right;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator> (const Duration& other)
{
  long left  = (long) (      mNegative ?       -mSecs :       mSecs);
  long right = (long) (other.mNegative ? -other.mSecs : other.mSecs);

  return left > right;
}

////////////////////////////////////////////////////////////////////////////////
Duration::~Duration ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::negative () const
{
  return mNegative;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::valid (const std::string& input) const
{
  std::string lower_input = lowerCase (input);

  // Assume the ordinal is 1, but look for an integer, just in case.
  double value = 1;
  Nibbler n (lower_input);
  n.skipAll (' ');
  n.getNumber (value);
  n.skipAll (' ');

  if (value < 0.0)
    value = -value;

  std::string units;
  n.getUntilEOS (units);

  // Auto complete against all supported durations.
  std::vector <std::string> supported;
  for (unsigned int i = 0; i < NUM_DURATIONS; ++i)
    supported.push_back (durations[i]);

  std::vector <std::string> matches;
  if (autoComplete (units, supported, matches) == 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Duration::parse (const std::string& input)
{
  std::string lower_input = lowerCase (input);

  // Assume the ordinal is 1, but look for an integer, just in case.
  double value = 1;
  Nibbler n (lower_input);
  n.skipAll (' ');
  n.getNumber (value);

  n.skipAll (' ');

  if (value < 0.0)
  {
    mNegative = true;
    value = -value;
  }
  else
    mNegative = false;

  std::string units;
  n.getUntilEOS (units);

  // Auto complete against all supported durations.
  std::vector <std::string> supported;
  for (unsigned int i = 0; i < NUM_DURATIONS; ++i)
    supported.push_back (durations[i]);

  mSecs = 0;
  std::vector <std::string> matches;
  if (autoComplete (units, supported, matches) == 1)
  {
    std::string match = matches[0];

         if (match == "biannual")                         mSecs = (int) (value * 86400 * 730);
    else if (match == "biyearly")                         mSecs = (int) (value * 86400 * 730);

    else if (match == "yearly")                           mSecs = (int) (value * 86400 * 365);
    else if (match == "annual")                           mSecs = (int) (value * 86400 * 365);
    else if (match == "years")                            mSecs = (int) (value * 86400 * 365);
    else if (match == "yrs")                              mSecs = (int) (value * 86400 * 365);
    else if (match == "y")                                mSecs = (int) (value * 86400 * 365);
    else if (match == "yearly")                           mSecs = (int) (value * 86400 * 365);
    else if (match == "annual")                           mSecs = (int) (value * 86400 * 365);

    else if (match == "semiannual")                       mSecs = (int) (value * 86400 * 183);

    else if (match == "bimonthly")                        mSecs = (int) (value * 86400 * 61);
    else if (match == "quarterly")                        mSecs = (int) (value * 86400 * 91);
    else if (match == "quarters")                         mSecs = (int) (value * 86400 * 91);
    else if (match == "qrtrs")                            mSecs = (int) (value * 86400 * 91);
    else if (match == "qtrs")                             mSecs = (int) (value * 86400 * 91);
    else if (match == "q")                                mSecs = (int) (value * 86400 * 91);

    else if (match == "monthly")                          mSecs = (int) (value * 86400 * 30);
    else if (match == "month")                            mSecs = (int) (value * 86400 * 30);
    else if (match == "months")                           mSecs = (int) (value * 86400 * 30);
    else if (match == "mnths")                            mSecs = (int) (value * 86400 * 30);
    else if (match == "mos")                              mSecs = (int) (value * 86400 * 30);
    else if (match == "mo")                               mSecs = (int) (value * 86400 * 30);
    else if (match == "mths")                             mSecs = (int) (value * 86400 * 30);

    else if (match == "biweekly")                         mSecs = (int) (value * 86400 * 14);
    else if (match == "fortnight")                        mSecs = (int) (value * 86400 * 14);

    else if (match == "weekly")                           mSecs = (int) (value * 86400 * 7);
    else if (match == "sennight")                         mSecs = (int) (value * 86400 * 7);
    else if (match == "weeks")                            mSecs = (int) (value * 86400 * 7);
    else if (match == "wks")                              mSecs = (int) (value * 86400 * 7);
    else if (match == "w")                                mSecs = (int) (value * 86400 * 7);

    else if (match == "daily")                            mSecs = (int) (value * 86400 * 1);
    else if (match == "day")                              mSecs = (int) (value * 86400 * 1);
    else if (match == "weekdays")                         mSecs = (int) (value * 86400 * 1);
    else if (match == "days")                             mSecs = (int) (value * 86400 * 1);
    else if (match == "d")                                mSecs = (int) (value * 86400 * 1);

    else if (match == "hours")                            mSecs = (int) (value * 3600);
    else if (match == "hrs")                              mSecs = (int) (value * 3600);
    else if (match == "h")                                mSecs = (int) (value * 3600);

    else if (match == "minutes")                          mSecs = (int) (value * 60);
    else if (match == "mins")                             mSecs = (int) (value * 60);
    else if (match == "min")                              mSecs = (int) (value * 60);
    else if (match == "m")                                mSecs = (int) (value * 60);

    else if (match == "seconds")                          mSecs = (int) value;
    else if (match == "secs")                             mSecs = (int) value;
    else if (match == "sec")                              mSecs = (int) value;
    else if (match == "s")                                mSecs = (int) value;

    else if (match == "-")                                mSecs = 0;
  }

  if (mSecs == 0)
    throw std::string ("The duration '") + input + "' was not recognized.";
}

////////////////////////////////////////////////////////////////////////////////
