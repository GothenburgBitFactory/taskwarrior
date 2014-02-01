////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Context.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <OldDuration.h>

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
  "hour",
  "hrs",
  "hr",
  "h",
  "minutes",
  "mins",
  "min",
  "monthly",
  "months",
  "month",
  "mnths",
  "mths",
  "mth",
  "mos",
  "mo",
  "quarterly",
  "quarters",
  "qrtrs",
  "qtrs",
  "qtr",
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
  "week",
  "wks",
  "wk",
  "w",
  "yearly",
  "years",
  "year",
  "yrs",
  "yr",
  "y",
  "-",
};

#define NUM_DURATIONS   (sizeof (durations)   / sizeof (durations[0]))

extern Context context;

////////////////////////////////////////////////////////////////////////////////
OldDuration::OldDuration ()
: _secs (0)
, _negative (false)
{
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::OldDuration (const OldDuration& other)
{
  _secs     = other._secs;
  _negative = other._negative;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::OldDuration (time_t input)
{
  if (input < 0)
  {
    _secs = -input;
    _negative = true;
  }
  else
  {
    _secs = input;
    _negative = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::OldDuration (const std::string& input)
: _secs (0)
, _negative (false)
{
  if (digitsOnly (input))
  {
    time_t value = (time_t) strtol (input.c_str (), NULL, 10);
    if (value == 0 || value > 60)
    {
      _secs = value;
      _negative = false;
      return;
    }
  }

  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::operator time_t () const
{
  return _secs;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::operator std::string () const
{
  std::stringstream s;
  s << (_negative ? - (long) _secs : (long) _secs);
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
OldDuration& OldDuration::operator= (const OldDuration& other)
{
  if (this != &other)
  {
    _secs     = other._secs;
    _negative = other._negative;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration OldDuration::operator- (const OldDuration& other)
{
  int left  =       _secs * (      _negative ? -1 : 1);
  int right = other._secs * (other._negative ? -1 : 1);

  left -= right;

  OldDuration result;
  result._secs = abs (left);
  result._negative = left < 0;

  return result;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration OldDuration::operator+ (const OldDuration& other)
{
  int left  =       _secs * (      _negative ? -1 : 1);
  int right = other._secs * (other._negative ? -1 : 1);

  left += right;

  OldDuration result;
  result._secs = abs (left);
  result._negative = left < 0;

  return result;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration& OldDuration::operator-= (const OldDuration& other)
{
  int left  =       _secs * (      _negative ? -1 : 1);
  int right = other._secs * (other._negative ? -1 : 1);

  left -= right;

  _secs = abs (left);
  _negative = left < 0;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration& OldDuration::operator+= (const OldDuration& other)
{
  int left  =       _secs * (      _negative ? -1 : 1);
  int right = other._secs * (other._negative ? -1 : 1);

  left += right;

  _secs = abs (left);
  _negative = left < 0;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string OldDuration::format () const
{
  char formatted[24];
  float days = (float) _secs / 86400.0;

  if (_secs >= 86400 * 365)
    sprintf (formatted, "%s%.1f yrs", (_negative ? "-" : ""), (days / 365));
  else if (_secs > 86400 * 84)
    sprintf (formatted, "%s%1d mth%s",
                        (_negative ? "-" : ""),  (int) (float) (days / 30),
                        ((int) (float) (days / 30) == 1 ? "" : "s"));
  else if (_secs > 86400 * 13)
    sprintf (formatted, "%s%d wk%s",
                        (_negative ? "-" : ""),  (int) (float) (days / 7.0),
                        ((int) (float) (days / 7.0) == 1 ? "" : "s"));
  else if (_secs >= 86400)
    sprintf (formatted, "%s%d day%s",
                        (_negative ? "-" : ""),  (int) days,
                        ((int) days == 1 ? "" : "s"));
  else if (_secs >= 3600)
    sprintf (formatted, "%s%d hr%s",
                        (_negative ? "-" : ""),  (int) (float) (_secs / 3600),
                        ((int) (float) (_secs / 3600) == 1 ? "" : "s"));
  else if (_secs >= 60)
    sprintf (formatted, "%s%d min%s",
                        (_negative ? "-" : ""),  (int) (float) (_secs / 60),
                        ((int) (float) (_secs / 60) == 1 ? "" : "s"));
  else if (_secs >= 1)
    sprintf (formatted, "%s%d sec%s",
                        (_negative ? "-" : ""),  (int) _secs,
                        ((int) _secs == 1 ? "" : "s"));
  else
    strcpy (formatted, "-"); // no i18n

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string OldDuration::formatCompact () const
{
  char formatted[24];
  float days = (float) _secs / 86400.0;

       if (_secs >= 86400 * 365) sprintf (formatted, "%s%.1fy", (_negative ? "-" : ""), (days / 365.0));
  else if (_secs >= 86400 * 84)  sprintf (formatted, "%s%1dmo", (_negative ? "-" : ""), (int) (days / 30));
  else if (_secs >= 86400 * 13)  sprintf (formatted, "%s%dwk",  (_negative ? "-" : ""), (int) (float) (days / 7.0));
  else if (_secs >= 86400)       sprintf (formatted, "%s%dd",   (_negative ? "-" : ""), (int) days);
  else if (_secs >= 3600)        sprintf (formatted, "%s%dh",   (_negative ? "-" : ""), (int) (_secs / 3600));
  else if (_secs >= 60)          sprintf (formatted, "%s%dm",   (_negative ? "-" : ""), (int) (_secs / 60));
  else if (_secs >= 1)           sprintf (formatted, "%s%ds",   (_negative ? "-" : ""), (int) _secs);
  else                           formatted[0] = '\0';

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string OldDuration::formatPrecise () const
{
  char formatted[24];

  int days    = _secs / 86400;
  int hours   = (_secs % 86400) / 3600;
  int minutes = (_secs % 3600) / 60;
  int seconds = _secs % 60;

  if (days > 0) sprintf (formatted, "%s%dd %d:%02d:%02d", (_negative ? "-" : ""), days, hours, minutes, seconds);
  else          sprintf (formatted, "%s%d:%02d:%02d", (_negative ? "-" : ""), hours, minutes, seconds);

  return std::string (formatted);
}


////////////////////////////////////////////////////////////////////////////////
std::string OldDuration::formatSeconds () const
{
  char formatted[24];
  sprintf (formatted, "%s%llusec", (_negative ? "-" : ""), (unsigned long long)_secs);
  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::operator< (const OldDuration& other)
{
  long left  = (long) (      _negative ?       -_secs :       _secs);
  long right = (long) (other._negative ? -other._secs : other._secs);

  return left < right;
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::operator<= (const OldDuration& other)
{
  long left  = (long) (      _negative ?       -_secs :       _secs);
  long right = (long) (other._negative ? -other._secs : other._secs);

  return left <= right;
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::operator> (const OldDuration& other)
{
  long left  = (long) (      _negative ?       -_secs :       _secs);
  long right = (long) (other._negative ? -other._secs : other._secs);

  return left > right;
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::operator>= (const OldDuration& other)
{
  long left  = (long) (      _negative ?       -_secs :       _secs);
  long right = (long) (other._negative ? -other._secs : other._secs);

  return left >= right;
}

////////////////////////////////////////////////////////////////////////////////
OldDuration::~OldDuration ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::negative () const
{
  return _negative;
}

////////////////////////////////////////////////////////////////////////////////
bool OldDuration::valid (const std::string& input)
{
  std::string lower_input = lowerCase (input);

  // Assume the ordinal is 1, but look for an integer, just in case.
  double value = 1;
  Nibbler n (lower_input);
  n.getNumber (value);

  if (value < 0.0)
    value = -value;

  std::string units;
  n.getUntilEOS (units);

  // Non-trivial value with no units means the duration is specified in
  // seconds, and therefore a time_t.  Consider it valid provided it is >= 60.
  if (value != 0.0  &&
      value >= 60.0 &&
      units == "")
    return true;

  // Auto complete against all supported durations.
  std::vector <std::string> supported;
  for (unsigned int i = 0; i < NUM_DURATIONS; ++i)
    supported.push_back (durations[i]);

  std::vector <std::string> matches;
  if (autoComplete (units,
                    supported,
                    matches,
                    context.config.getInteger ("abbreviation.minimum")) == 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void OldDuration::parse (const std::string& input)
{
  std::string lower_input = lowerCase (input);

  // Assume the ordinal is 1, but look for an integer, just in case.
  double value = 1;
  Nibbler n (lower_input);
  n.getNumber (value);

  if (value < 0.0)
  {
    _negative = true;
    value = -value;
  }
  else
    _negative = false;

  // If no units are provided, assume seconds.
  std::string units;
  if (n.depleted () && value >= 60)
  {
    _secs = (long) value;
    return;
  }

  n.getUntilEOS (units);

  // Auto complete against all supported durations.
  std::vector <std::string> supported;
  for (unsigned int i = 0; i < NUM_DURATIONS; ++i)
    supported.push_back (durations[i]);

  _secs = 0;
  std::vector <std::string> matches;
  if (autoComplete (units,
                    supported,
                    matches,
                    context.config.getInteger ("abbreviation.minimum")) == 1)
  {
    std::string match = matches[0];

         if (match == "biannual")                         _secs = (int) (value * 86400 * 730);
    else if (match == "biyearly")                         _secs = (int) (value * 86400 * 730);

    else if (match == "yearly")                           _secs = (int) (value * 86400 * 365);
    else if (match == "annual")                           _secs = (int) (value * 86400 * 365);
    else if (match == "years")                            _secs = (int) (value * 86400 * 365);
    else if (match == "year")                             _secs = (int) (value * 86400 * 365);
    else if (match == "yrs")                              _secs = (int) (value * 86400 * 365);
    else if (match == "yr")                               _secs = (int) (value * 86400 * 365);
    else if (match == "y")                                _secs = (int) (value * 86400 * 365);

    else if (match == "semiannual")                       _secs = (int) (value * 86400 * 183);

    else if (match == "bimonthly")                        _secs = (int) (value * 86400 * 61);
    else if (match == "quarterly")                        _secs = (int) (value * 86400 * 91);
    else if (match == "quarters")                         _secs = (int) (value * 86400 * 91);
    else if (match == "qrtrs")                            _secs = (int) (value * 86400 * 91);
    else if (match == "qtrs")                             _secs = (int) (value * 86400 * 91);
    else if (match == "qtr")                              _secs = (int) (value * 86400 * 91);
    else if (match == "q")                                _secs = (int) (value * 86400 * 91);

    else if (match == "monthly")                          _secs = (int) (value * 86400 * 30);
    else if (match == "month")                            _secs = (int) (value * 86400 * 30);
    else if (match == "months")                           _secs = (int) (value * 86400 * 30);
    else if (match == "mnths")                            _secs = (int) (value * 86400 * 30);
    else if (match == "mos")                              _secs = (int) (value * 86400 * 30);
    else if (match == "mo")                               _secs = (int) (value * 86400 * 30);
    else if (match == "mths")                             _secs = (int) (value * 86400 * 30);
    else if (match == "mth")                              _secs = (int) (value * 86400 * 30);
    else if (match == "m")                                _secs = (int) (value * 86400 * 30);

    else if (match == "biweekly")                         _secs = (int) (value * 86400 * 14);
    else if (match == "fortnight")                        _secs = (int) (value * 86400 * 14);

    else if (match == "weekly")                           _secs = (int) (value * 86400 * 7);
    else if (match == "sennight")                         _secs = (int) (value * 86400 * 7);
    else if (match == "weeks")                            _secs = (int) (value * 86400 * 7);
    else if (match == "week")                             _secs = (int) (value * 86400 * 7);
    else if (match == "wks")                              _secs = (int) (value * 86400 * 7);
    else if (match == "wk")                               _secs = (int) (value * 86400 * 7);
    else if (match == "w")                                _secs = (int) (value * 86400 * 7);

    else if (match == "daily")                            _secs = (int) (value * 86400 * 1);
    else if (match == "day")                              _secs = (int) (value * 86400 * 1);
    else if (match == "weekdays")                         _secs = (int) (value * 86400 * 1);
    else if (match == "days")                             _secs = (int) (value * 86400 * 1);
    else if (match == "d")                                _secs = (int) (value * 86400 * 1);

    else if (match == "hours")                            _secs = (int) (value * 3600);
    else if (match == "hour")                             _secs = (int) (value * 3600);
    else if (match == "hrs")                              _secs = (int) (value * 3600);
    else if (match == "hr")                               _secs = (int) (value * 3600);
    else if (match == "h")                                _secs = (int) (value * 3600);

    else if (match == "minutes")                          _secs = (int) (value * 60);
    else if (match == "mins")                             _secs = (int) (value * 60);
    else if (match == "min")                              _secs = (int) (value * 60);

    else if (match == "seconds")                          _secs = (int) value;
    else if (match == "secs")                             _secs = (int) value;
    else if (match == "sec")                              _secs = (int) value;
    else if (match == "s")                                _secs = (int) value;

    else if (match == "-")                                _secs = 0;
  }

  if (_secs == 0)
    throw ::format (STRING_DURATION_UNRECOGNIZED, input);
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> OldDuration::get_units ()
{
  std::vector <std::string> units;
  for (unsigned int i = 0; i < NUM_DURATIONS; ++i)
    if (strcmp (durations[i], "-"))
      units.push_back (durations[i]);

  return units;
}

////////////////////////////////////////////////////////////////////////////////
