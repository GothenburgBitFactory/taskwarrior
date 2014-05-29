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

#include <cmake.h>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Nibbler.h>
#include <Lexer.h>
#include <Duration.h>
#include <text.h>

#define DAY    86400
#define HOUR    3600
#define MINUTE    60
#define SECOND     1

static struct
{
  std::string unit;
  int seconds;
} durations[] =
{
  // These are sorted by first character, then length, so that Nibbler::getOneOf
  // returns a maximal match.
  {"annual",     365 * DAY},
  {"biannual",   730 * DAY},
  {"bimonthly",   61 * DAY},
  {"biweekly",    14 * DAY},
  {"biyearly",   730 * DAY},
  {"daily",        1 * DAY},
  {"days",         1 * DAY},
  {"day",          1 * DAY},
  {"d",            1 * DAY},
  {"fortnight",   14 * DAY},
  {"hours",        1 * HOUR},
  {"hour",         1 * HOUR},
  {"hrs",          1 * HOUR},           // Deprecate
  {"hr",           1 * HOUR},           // Deprecate
  {"h",            1 * HOUR},
  {"minutes",      1 * MINUTE},
  {"minute",       1 * MINUTE},
  {"mins",         1 * MINUTE},         // Deprecate
  {"min",          1 * MINUTE},
  {"monthly",     30 * DAY},
  {"months",      30 * DAY},
  {"month",       30 * DAY},
  {"mnths",       30 * DAY},            // Deprecate
  {"mths",        30 * DAY},            // Deprecate
  {"mth",         30 * DAY},            // Deprecate
  {"mos",         30 * DAY},            // Deprecate
  {"mo",          30 * DAY},
  {"quarterly",   91 * DAY},
  {"quarters",    91 * DAY},
  {"quarter",     91 * DAY},
  {"qrtrs",       91 * DAY},            // Deprecate
  {"qtrs",        91 * DAY},            // Deprecate
  {"qtr",         91 * DAY},            // Deprecate
  {"q",           91 * DAY},
  {"semiannual", 183 * DAY},
  {"sennight",    14 * DAY},
  {"seconds",      1 * SECOND},
  {"second",       1 * SECOND},
  {"secs",         1 * SECOND},         // Deprecate
  {"sec",          1 * SECOND},         // Deprecate
  {"s",            1 * SECOND},
  {"weekdays",         DAY},
  {"weekly",       7 * DAY},
  {"weeks",        7 * DAY},
  {"week",         7 * DAY},
  {"wks",          7 * DAY},            // Deprecate
  {"wk",           7 * DAY},            // Deprecate
  {"w",            7 * DAY},
  {"yearly",     365 * DAY},
  {"years",      365 * DAY},
  {"year",       365 * DAY},
  {"yrs",        365 * DAY},            // Deprecate
  {"yr",         365 * DAY},            // Deprecate
  {"y",          365 * DAY},
};

#define NUM_DURATIONS (sizeof (durations) / sizeof (durations[0]))

////////////////////////////////////////////////////////////////////////////////
Duration::Duration ()
: _secs (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (time_t input)
: _secs (input)
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const std::string& input)
: _secs (0)
{
  if (digitsOnly (input))
  {
    time_t value = (time_t) strtol (input.c_str (), NULL, 10);
    if (value == 0 || value > 60)
    {
      _secs = value;
      return;
    }
  }

  std::string::size_type idx = 0;
  parse (input, idx);
}

////////////////////////////////////////////////////////////////////////////////
Duration::~Duration ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator< (const Duration& other)
{
  return _secs < other._secs;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator> (const Duration& other)
{
  return _secs > other._secs;
}

////////////////////////////////////////////////////////////////////////////////
Duration& Duration::operator= (const Duration& other)
{
  if (this != &other)
    _secs = other._secs;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator time_t () const
{
  return _secs;
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator std::string () const
{
  std::stringstream s;
  s << _secs;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::format () const
{
  char formatted[24];
  float days = (float) _secs / 86400.0;

  if (_secs >= 86400 * 365)
    sprintf (formatted, "%.1f yrs", (days / 365));
  else if (_secs > 86400 * 84)
    sprintf (formatted, "%1d mth%s",
                        (int) (float) (days / 30),
                        ((int) (float) (days / 30) == 1 ? "" : "s"));
  else if (_secs > 86400 * 13)
    sprintf (formatted, "%d wk%s",
                        (int) (float) (days / 7.0),
                        ((int) (float) (days / 7.0) == 1 ? "" : "s"));
  else if (_secs >= 86400)
    sprintf (formatted, "%d day%s",
                        (int) days,
                        ((int) days == 1 ? "" : "s"));
  else if (_secs >= 3600)
    sprintf (formatted, "%d hr%s",
                        (int) (float) (_secs / 3600),
                        ((int) (float) (_secs / 3600) == 1 ? "" : "s"));
  else if (_secs >= 60)
    sprintf (formatted, "%d min%s",
                        (int) (float) (_secs / 60),
                        ((int) (float) (_secs / 60) == 1 ? "" : "s"));
  else if (_secs >= 1)
    sprintf (formatted, "%d sec%s",
                        (int) _secs,
                        ((int) _secs == 1 ? "" : "s"));
  else
    strcpy (formatted, "-"); // no i18n

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::formatCompact () const
{
  char formatted[24];
  float days = (float) _secs / 86400.0;

       if (_secs >= 86400 * 365) sprintf (formatted, "%.1fy", (days / 365.0));
  else if (_secs >= 86400 * 84)  sprintf (formatted, "%1dmo", (int) (days / 30));
  else if (_secs >= 86400 * 13)  sprintf (formatted, "%dwk",  (int) (float) (days / 7.0));
  else if (_secs >= 86400)       sprintf (formatted, "%dd",   (int) days);
  else if (_secs >= 3600)        sprintf (formatted, "%dh",   (int) (_secs / 3600));
  else if (_secs >= 60)          sprintf (formatted, "%dm",   (int) (_secs / 60));
  else if (_secs >= 1)           sprintf (formatted, "%ds",   (int) _secs);
  else                           formatted[0] = '\0';

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::formatPrecise () const
{
  char formatted[24];

  int days    = _secs / 86400;
  int hours   = (_secs % 86400) / 3600;
  int minutes = (_secs % 3600) / 60;
  int seconds = _secs % 60;

  if (days > 0) sprintf (formatted, "%dd %d:%02d:%02d", days, hours, minutes, seconds);
  else          sprintf (formatted, "%d:%02d:%02d",     hours, minutes, seconds);

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::formatSeconds () const
{
  char formatted[24];
  sprintf (formatted, "%llusec", (unsigned long long)_secs);
  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::formatISO () const
{
  if (_secs)
  {
    time_t t = _secs;
    int seconds = t % 60; t /= 60;
    int minutes = t % 60; t /= 60;
    int hours   = t % 24; t /= 24;
    int days    = t % 30; t /= 30;
    int months  = t % 12; t /= 12;
    int years   = t;

    std::stringstream s;
    s << 'P';
    if (years)  s << years  << 'Y';
    if (months) s << months << 'M';
    if (days)   s << days   << 'D';

    if (hours || minutes || seconds)
    {
      s << 'T';
      if (hours)   s << hours   << 'H';
      if (minutes) s << minutes << 'M';
      if (seconds) s << seconds << 'S';
    }

    return s.str ();
  }
  else
  {
    return "P0S";
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::parse (const std::string& input, std::string::size_type& start)
{
  std::string::size_type original_start = start;
  Nibbler n (input.substr (start));

  std::vector <std::string> units;
  for (int i = 0; i < NUM_DURATIONS; i++)
    units.push_back (durations[i].unit);

  std::string number;
  std::string unit;
  if ((n.getNumber (number) && n.skipWS () && n.getOneOf (units, unit)) ||
                                              n.getOneOf (units, unit))
  {
    if (n.depleted () ||
        Lexer::is_ws (n.next ()))
    {
      start = original_start + n.cursor ();
      double quantity = (number == "")
                          ? 1.0
                          : strtod (number.c_str (), NULL);

      // Linear lookup - should be logarithmic.
      double seconds = 1;
      for (int i = 0; i < NUM_DURATIONS; i++)
      {
        if (durations[i].unit == unit)
        {
          seconds = durations[i].seconds;
          _secs = static_cast <int> (quantity * static_cast <double> (seconds));
          return true;
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Duration::clear ()
{
  _secs = 0;
}

////////////////////////////////////////////////////////////////////////////////
