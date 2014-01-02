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
#include <stdlib.h>
#include <Nibbler.h>
#include <Lexer.h>
#include <Duration.h>

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
Duration::~Duration ()
{
}

////////////////////////////////////////////////////////////////////////////////
Duration::operator time_t () const
{
  return _secs;
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
