////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
  bool standalone;
} durations[] =
{
  // These are sorted by first character, then length, so that Nibbler::getOneOf
  // returns a maximal match.
  {"annual",     365 * DAY,    true},
  {"biannual",   730 * DAY,    true},
  {"bimonthly",   61 * DAY,    true},
  {"biweekly",    14 * DAY,    true},
  {"biyearly",   730 * DAY,    true},
  {"daily",        1 * DAY,    true},
  {"days",         1 * DAY,    false},
  {"day",          1 * DAY,    true},
  {"d",            1 * DAY,    false},
  {"fortnight",   14 * DAY,    true},
  {"hours",        1 * HOUR,   false},
  {"hour",         1 * HOUR,   true},
  {"hrs",          1 * HOUR,   true},
  {"hr",           1 * HOUR,   true},
  {"h",            1 * HOUR,   false},
  {"minutes",      1 * MINUTE, false},
  {"minute",       1 * MINUTE, false},
  {"mins",         1 * MINUTE, false},
  {"min",          1 * MINUTE, false},
  {"monthly",     30 * DAY,    true},
  {"months",      30 * DAY,    false},
  {"month",       30 * DAY,    true},
  {"mnths",       30 * DAY,    false},
  {"mths",        30 * DAY,    false},
  {"mth",         30 * DAY,    false},
  {"mos",         30 * DAY,    false},
  {"mo",          30 * DAY,    false},
  {"m",           30 * DAY,    false},
  {"quarterly",   91 * DAY,    true},
  {"quarters",    91 * DAY,    false},
  {"quarter",     91 * DAY,    true},
  {"qrtrs",       91 * DAY,    false},
  {"qtrs",        91 * DAY,    false},
  {"qtr",         91 * DAY,    false},
  {"q",           91 * DAY,    false},
  {"semiannual", 183 * DAY,    true},
  {"sennight",    14 * DAY,    false},
  {"seconds",      1 * SECOND, false},
  {"second",       1 * SECOND, true},
  {"secs",         1 * SECOND, true},
  {"sec",          1 * SECOND, true},
  {"s",            1 * SECOND, false},
  {"weekdays",     1 * DAY,    true},
  {"weekly",       7 * DAY,    true},
  {"weeks",        7 * DAY,    false},
  {"week",         7 * DAY,    true},
  {"wks",          7 * DAY,    true},
  {"wk",           7 * DAY,    true},
  {"w",            7 * DAY,    false},
  {"yearly",     365 * DAY,    true},
  {"years",      365 * DAY,    false},
  {"year",       365 * DAY,    true},
  {"yrs",        365 * DAY,    true},
  {"yr",         365 * DAY,    true},
  {"y",          365 * DAY,    false},
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
  if (Lexer::isAllDigits (input))
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
    sprintf (formatted, "%.1f year%s",
                        (days / 365),
                        ((int) (float) (days / 365) == 1 ? "" : "s"));
  else if (_secs > 86400 * 84)
    sprintf (formatted, "%1d month%s",
                        (int) (float) (days / 30),
                        ((int) (float) (days / 30) == 1 ? "" : "s"));
  else if (_secs > 86400 * 13)
    sprintf (formatted, "%d week%s",
                        (int) (float) (days / 7.0),
                        ((int) (float) (days / 7.0) == 1 ? "" : "s"));
  else if (_secs >= 86400)
    sprintf (formatted, "%d day%s",
                        (int) days,
                        ((int) days == 1 ? "" : "s"));
  else if (_secs >= 3600)
    sprintf (formatted, "%d hour%s",
                        (int) (float) (_secs / 3600),
                        ((int) (float) (_secs / 3600) == 1 ? "" : "s"));
  else if (_secs >= 60)
    sprintf (formatted, "%d minute%s",
                        (int) (float) (_secs / 60),
                        ((int) (float) (_secs / 60) == 1 ? "" : "s"));
  else if (_secs >= 1)
    sprintf (formatted, "%d second%s",
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
  else if (_secs >= 86400 * 13)  sprintf (formatted, "%dw",   (int) (float) (days / 7.0));
  else if (_secs >= 86400)       sprintf (formatted, "%dd",   (int) days);
  else if (_secs >= 3600)        sprintf (formatted, "%dh",   (int) (_secs / 3600));
  else if (_secs >= 60)          sprintf (formatted, "%dmin", (int) (_secs / 60));
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
  sprintf (formatted, "%llus", (unsigned long long)_secs);
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

  // Static and so preserved between calls.
  static std::vector <std::string> units;
  if (units.size () == 0)
    for (int i = 0; i < NUM_DURATIONS; i++)
      units.push_back (durations[i].unit);

  std::string number;
  std::string unit;

  if (n.getOneOf (units, unit))
  {
    if (n.depleted () ||
        Lexer::isWhitespace (n.next ()))
    {
      start = original_start + n.cursor ();

      // Linear lookup - should be logarithmic.
      double seconds = 1;
      for (int i = 0; i < NUM_DURATIONS; i++)
      {
        if (durations[i].unit == unit &&
            durations[i].standalone == true)
        {
          _secs = static_cast <int> (durations[i].seconds);
          return true;
        }
      }
    }
  }

  else if (n.getNumber (number))
  {
    n.skipWS ();
    if (n.getOneOf (units, unit))
    {
      if (n.depleted () ||
          Lexer::isWhitespace (n.next ()))
      {
        start = original_start + n.cursor ();
        double quantity = strtod (number.c_str (), NULL);

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
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Duration::clear ()
{
  _secs = 0;
}

////////////////////////////////////////////////////////////////////////////////
