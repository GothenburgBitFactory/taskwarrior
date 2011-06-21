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

#define L10N                                           // Localization complete.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <inttypes.h>
#include <Nibbler.h>
#include <Date.h>
#include <RegX.h>

const char* c_digits = "0123456789";

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler ()
: mInput ("")
, mLength (0)
, mCursor (0)
, mSaved (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const char* input)
: mInput (input)
, mLength (strlen (input))
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const std::string& input)
: mInput (input)
, mLength (input.length ())
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const Nibbler& other)
{
  mInput  = other.mInput;
  mLength = other.mLength;
  mCursor = other.mCursor;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler& Nibbler::operator= (const Nibbler& other)
{
  if (this != &other)
  {
    mInput  = other.mInput;
    mLength = other.mLength;
    mCursor = other.mCursor;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::~Nibbler ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Extract up until the next c (but not including) or EOS.
bool Nibbler::getUntil (char c, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find (c, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntil (const std::string& terminator, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find (terminator, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilRx (const std::string& regex, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string modified_regex;
    if (regex[0] != '(')
      modified_regex = "(" + regex + ")";
    else
      modified_regex = regex;

    RegX r (modified_regex, true);
    std::vector <int> start;
    std::vector <int> end;
    if (r.match (start, end, mInput.substr (mCursor)))
    {
      result = mInput.substr (mCursor, start[0]);
      mCursor += start[0];
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilOneOf (const std::string& chars, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_of (chars, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilWS (std::string& result)
{
  return this->getUntilOneOf (" \t\r\n\f", result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOL (std::string& result)
{
  return getUntil ('\n', result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOS (std::string& result)
{
  if (mCursor < mLength)
  {
    result = mInput.substr (mCursor);
    mCursor = mLength;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getQuoted (
  char c,
  std::string& result,
  bool quote /* = false */)
{
  bool inquote = false;
  bool inescape = false;
  char current = 0;
  char previous = 0;
  result = "";

  if (mCursor >= mLength ||
      mInput[mCursor] != c)
  {
    return false;
  }

  for (std::string::size_type i = mCursor; i < mLength; ++i)
  {
    previous = current;
    current = mInput[i];

    if (current == '\\' && !inescape)
    {
      inescape = true;
      continue;
    }

    if (current == c && !inescape)
    {
      if (quote)
        result += current;

      if (!inquote)
      {
        inquote = true;
      }
      else
      {
        mCursor = i + 1;
        return true;
      }
    }
    else
    {
      result += current;
      inescape = false;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDigit (int& result)
{
  if (mCursor < mLength &&
      isdigit (mInput[mCursor]))
  {
    result = mInput[mCursor++] - '0';
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getInt (int& result)
{
  std::string::size_type i = mCursor;

  if (i < mLength)
  {
    if (mInput[i] == '-')
      ++i;
    else if (mInput[i] == '+')
      ++i;
  }

  // TODO Potential for use of find_first_not_of
  while (i < mLength && isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = strtoimax (mInput.substr (mCursor, i - mCursor).c_str (), NULL, 10);
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getHex (int& result)
{
  std::string::size_type i = mCursor;

  if (i < mLength)
  {
    if (mInput[i] == '-')
      ++i;
    else if (mInput[i] == '+')
      ++i;
  }

  // TODO Potential for use of find_first_not_of
  while (i < mLength && isxdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = strtoimax (mInput.substr (mCursor, i - mCursor).c_str (), NULL, 16);
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUnsignedInt (int& result)
{
  std::string::size_type i = mCursor;
  // TODO Potential for use of find_first_not_of
  while (i < mLength && isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = strtoimax (mInput.substr (mCursor, i - mCursor).c_str (), NULL, 10);
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// number:
//   int frac? exp?
// 
// int:
//   -? digit+
// 
// frac:
//   . digit+
// 
// exp:
//   e digit+
// 
// e:
//   e|E (+|-)?
// 
bool Nibbler::getNumber (double& result)
{
  std::string::size_type i = mCursor;

  // [+-]?
  if (i < mLength && mInput[i] == '-')
    ++i;

  // digit+
  if (i < mLength && isdigit (mInput[i]))
  {
    ++i;

    while (i < mLength && isdigit (mInput[i]))
      ++i;

    // ( . digit+ )?
    if (i < mLength && mInput[i] == '.')
    {
      ++i;

      while (i < mLength && isdigit (mInput[i]))
        ++i;
    }

    // ( [eE] [+-]? digit+ )?
    if (i < mLength && (mInput[i] == 'e' || mInput[i] == 'E'))
    {
      ++i;

      if (i < mLength && (mInput[i] == '+' || mInput[i] == '-'))
        ++i;

      if (i < mLength && isdigit (mInput[i]))
      {
        ++i;

        while (i < mLength && isdigit (mInput[i]))
          ++i;

        result = strtof (mInput.substr (mCursor, i - mCursor).c_str (), NULL);
        mCursor = i;
        return true;
      }

      return false;
    }

    result = strtof (mInput.substr (mCursor, i - mCursor).c_str (), NULL);
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getLiteral (const std::string& literal)
{
  if (mCursor < mLength &&
      mInput.find (literal, mCursor) == mCursor)
  {
    mCursor += literal.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getRx (const std::string& regex, std::string& result)
{
  if (mCursor < mLength)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    RegX r (modified_regex, true);
    std::vector <std::string> results;
    if (r.match (results, mInput.substr (mCursor)))
    {
      result = results[0];
      mCursor += result.length ();
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUUID (std::string& result)
{
  std::string::size_type i = mCursor;

  if (i < mLength &&
      mLength - i >= 36)
  {
    // 88888888-4444-4444-4444-cccccccccccc
    if (isxdigit (mInput[i + 0]) &&
        isxdigit (mInput[i + 1]) &&
        isxdigit (mInput[i + 2]) &&
        isxdigit (mInput[i + 3]) &&
        isxdigit (mInput[i + 4]) &&
        isxdigit (mInput[i + 5]) &&
        isxdigit (mInput[i + 6]) &&
        isxdigit (mInput[i + 7]) &&
        mInput[i + 8] == '-'     &&
        isxdigit (mInput[i + 9]) &&
        isxdigit (mInput[i + 10]) &&
        isxdigit (mInput[i + 11]) &&
        isxdigit (mInput[i + 12]) &&
        mInput[i + 13] == '-'     &&
        isxdigit (mInput[i + 14]) &&
        isxdigit (mInput[i + 15]) &&
        isxdigit (mInput[i + 16]) &&
        isxdigit (mInput[i + 17]) &&
        mInput[i + 18] == '-'     &&
        isxdigit (mInput[i + 19]) &&
        isxdigit (mInput[i + 20]) &&
        isxdigit (mInput[i + 21]) &&
        isxdigit (mInput[i + 22]) &&
        mInput[i + 23] == '-'     &&
        isxdigit (mInput[i + 24]) &&
        isxdigit (mInput[i + 25]) &&
        isxdigit (mInput[i + 26]) &&
        isxdigit (mInput[i + 27]) &&
        isxdigit (mInput[i + 28]) &&
        isxdigit (mInput[i + 29]) &&
        isxdigit (mInput[i + 30]) &&
        isxdigit (mInput[i + 31]) &&
        isxdigit (mInput[i + 32]) &&
        isxdigit (mInput[i + 33]) &&
        isxdigit (mInput[i + 34]) &&
        isxdigit (mInput[i + 35]))
    {
      result = mInput.substr (mCursor, 36);
      mCursor = i + 36;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// 19980119T070000Z =  YYYYMMDDThhmmssZ
bool Nibbler::getDateISO (time_t& t)
{
  std::string::size_type i = mCursor;

  if (i < mLength &&
      mLength - i >= 16)
  {
    if (isdigit (mInput[i + 0]) &&
        isdigit (mInput[i + 1]) &&
        isdigit (mInput[i + 2]) &&
        isdigit (mInput[i + 3]) &&
        isdigit (mInput[i + 4]) &&
        isdigit (mInput[i + 5]) &&
        isdigit (mInput[i + 6]) &&
        isdigit (mInput[i + 7]) &&
        mInput[i + 8] == 'T' &&
        isdigit (mInput[i + 9]) &&
        isdigit (mInput[i + 10]) &&
        isdigit (mInput[i + 11]) &&
        isdigit (mInput[i + 12]) &&
        isdigit (mInput[i + 13]) &&
        isdigit (mInput[i + 14]) &&
        mInput[i + 15] == 'Z')
    {
      mCursor += 16;

      int year   = (mInput[i + 0] - '0') * 1000
                 + (mInput[i + 1] - '0') *  100
                 + (mInput[i + 2] - '0') *   10
                 + (mInput[i + 3] - '0');

      int month  = (mInput[i + 4] - '0') * 10
                 + (mInput[i + 5] - '0');

      int day    = (mInput[i + 6] - '0') * 10
                 + (mInput[i + 7] - '0');

      int hour   = (mInput[i + 9] - '0') * 10
                 + (mInput[i + 10] - '0');

      int minute = (mInput[i + 11] - '0') * 10
                 + (mInput[i + 12] - '0');

      int second = (mInput[i + 13] - '0') * 10
                 + (mInput[i + 14] - '0');

      // Convert to epoch.
      struct tm tms = {0};
      tms.tm_isdst = -1;   // Requests that mktime determine summer time effect.
      tms.tm_mday  = day;
      tms.tm_mon   = month - 1;
      tms.tm_year  = year - 1900;
      tms.tm_hour  = hour;
      tms.tm_min   = minute;
      tms.tm_sec   = second;

      t = timegm (&tms);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDate (const std::string& format, time_t& t)
{
  std::string::size_type i = mCursor;

  int month  = 0;
  int day    = 0;
  int year   = -1;   // So we can check later.
  int hour   = 0;
  int minute = 0;
  int second = 0;

  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    case 'm':
      if (i + 2 <= mLength &&
          (mInput[i + 0] == '0' || mInput[i + 0] == '1') &&
          isdigit (mInput[i + 1]))
      {
        month = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else if (i + 1 <= mLength &&
               isdigit (mInput[i + 0]))
      {
        month = mInput[i] - '0';
        i += 1;
      }
      else
        return false;
      break;

    case 'd':
      if (i + 2 <= mLength        &&
          isdigit (mInput[i + 1]) &&
          isdigit (mInput[i + 1]))
      {
        day = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else if (i + 1 <= mLength &&
               isdigit (mInput[i + 0]))
      {
        day = mInput[i] - '0';
        i += 1;
      }
      else
        return false;
      break;

    case 'y':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        year = 2000 + atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'M':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        month = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'D':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        day = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    // Merely parse, not extract.
    case 'V':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        day = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'Y':
      if (i + 4 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]) &&
          isdigit (mInput[i + 2]) &&
          isdigit (mInput[i + 3]))
      {
        year = atoi (mInput.substr (i, 4).c_str ());
        i += 4;
      }
      else
        return false;
      break;

    // Merely parse, not extract.
    case 'a':
      if (i + 3 <= mLength          &&
          ! isdigit (mInput[i + 0]) &&
          ! isdigit (mInput[i + 1]) &&
          ! isdigit (mInput[i + 2]))
        i += 3;
      else
        return false;
      break;

    // Merely parse, not extract.
    case 'b':
      if (i + 3 <= mLength          &&
          ! isdigit (mInput[i + 0]) &&
          ! isdigit (mInput[i + 1]) &&
          ! isdigit (mInput[i + 2]))
      {
        month = Date::monthOfYear (mInput.substr (i, 3).c_str());
        i += 3;
      }
      else
        return false;
      break;

    // Merely parse, not extract.
    case 'A':
      if (i + 3 <= mLength          &&
          ! isdigit (mInput[i + 0]) &&
          ! isdigit (mInput[i + 1]) &&
          ! isdigit (mInput[i + 2]))
        i += Date::dayName (Date::dayOfWeek (mInput.substr (i, 3).c_str ())).size ();
      else
        return false;
      break;

    case 'B':
      if (i + 3 <= mLength          &&
          ! isdigit (mInput[i + 0]) &&
          ! isdigit (mInput[i + 1]) &&
          ! isdigit (mInput[i + 2]))
      {
        month = Date::monthOfYear (mInput.substr (i, 3).c_str ());
        i += Date::monthName (month).size ();
      }
      else
        return false;
      break;

    case 'h':
      if (i + 2 <= mLength &&
          (mInput[i + 0] == '0' || mInput[i + 0] == '1') &&
          isdigit (mInput[i + 1]))
      {
        hour = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else if (i + 1 <= mLength &&
               isdigit (mInput[i + 0]))
      {
        hour = atoi (mInput.substr (i, 1).c_str ());
        i += 1;
      }
      else
        return false;
      break;

    case 'H':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        hour = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'N':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        minute = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'S':
      if (i + 2 <= mLength &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]))
      {
        second = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else
        return false;
      break;

    case 'j':
      if (i + 3 <= mLength        &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]) &&
          isdigit (mInput[i + 2]))
      {
        day = atoi (mInput.substr (i, 3).c_str ());
        i += 3;
      }
      else if (i + 2 <= mLength        &&
               isdigit (mInput[i + 0]) &&
               isdigit (mInput[i + 1]))
      {
        day = atoi (mInput.substr (i, 2).c_str ());
        i += 2;
      }
      else if (i + 1 <= mLength &&
               isdigit (mInput[i + 0]))
      {
        day = atoi (mInput.substr (i, 1).c_str ());
        i += 1;
      }
      else
        return false;
      break;

    case 'J':
      if (i + 3 <= mLength        &&
          isdigit (mInput[i + 0]) &&
          isdigit (mInput[i + 1]) &&
          isdigit (mInput[i + 2]))
      {
        day = atoi (mInput.substr (i, 3).c_str ());
        i += 3;
      }
      else
        return false;
      break;

    default:
      if (i + 1 <= mLength &&
          mInput[i] == format[f])
        ++i;
      else
        return false;
      break;
    }
  }

  // Default the year to the current year, for formats that lack Y/y.
  if (year == -1)
  {
    time_t now = time (NULL);
    struct tm* default_year = localtime (&now);
    year = default_year->tm_year + 1900;
  }

  // Convert to epoch.
  struct tm tms = {0};
  tms.tm_isdst = -1;   // Requests that mktime determine summer time effect.

  if (month == 0 && day >= 0 && day <= 365)
  {
    tms.tm_yday  = day;
    tms.tm_mon   = 0;

    if (! Date::valid (day, year))
      return false;
  }
  else
  {
    tms.tm_mday  = day;
    tms.tm_mon   = month > 0 ? month - 1 : 0;

    if (! Date::valid (month, day, year))
      return false;
  }

  tms.tm_year  = year - 1900;
  tms.tm_hour  = hour;
  tms.tm_min   = minute;
  tms.tm_sec   = second;

  t = mktime (&tms);
  mCursor = i;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getOneOf (
  const std::vector <std::string>& options,
  std::string& found)
{
  std::vector <std::string>::const_iterator option;
  for (option = options.begin (); option != options.end (); ++option)
  {
    if (getLiteral (*option))
    {
      found = *option;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDOM (std::string& found)
{
  std::string::size_type i     = mCursor;
  std::string::size_type start = mCursor;

  while (   isdigit (mInput[i]) ||
            mInput[i] == '.'    ||
            mInput[i] == '-'    ||
            mInput[i] == '_'    ||
         (! ispunct (mInput[i]) &&
          ! isspace (mInput[i])))
  {
    ++i;
  }

  if (i > mCursor)
  {
    found = mInput.substr (start, i - start);

    // If found is simple a number, then it is not a DOM reference.
    double d;
    Nibbler exclusion (found);
    if (exclusion.getNumber (d) && exclusion.depleted ())
      return false;

    int in;
    if (exclusion.getInt (in) && exclusion.depleted ())
      return false;

    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipN (const int quantity /* = 1 */)
{
  if (mCursor < mLength &&
      mCursor <= mLength - quantity)
  {
    mCursor += quantity;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skip (char c)
{
  if (mCursor < mLength &&
      mInput[mCursor] == c)
  {
    ++mCursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAll (char c)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_not_of (c, mCursor);
    if (i == mCursor)
      return false;

    if (i == std::string::npos)
      mCursor = mLength;  // Yes, off the end.
    else
      mCursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipWS ()
{
  return this->skipAllOneOf (" \t\n\r\f");
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipRx (const std::string& regex)
{
  if (mCursor < mLength)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    RegX r (modified_regex, true);
    std::vector <std::string> results;
    if (r.match (results, mInput.substr (mCursor)))
    {
      mCursor += results[0].length ();
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAllOneOf (const std::string& chars)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_not_of (chars, mCursor);
    if (i == mCursor)
      return false;

    if (i == std::string::npos)
      mCursor = mLength;  // Yes, off the end.
    else
      mCursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
char Nibbler::next ()
{
  if (mCursor < mLength)
    return mInput[mCursor];

  return '\0';
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Nibbler::cursor ()
{
  return mCursor;
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
std::string Nibbler::next (const int quantity)
{
  if (           mCursor  <  mLength &&
      (unsigned) quantity <= mLength &&
                 mCursor  <= mLength - quantity)
    return mInput.substr (mCursor, quantity);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Nibbler::save ()
{
  mSaved = mCursor;
}

////////////////////////////////////////////////////////////////////////////////
void Nibbler::restore ()
{
  mCursor = mSaved;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::depleted ()
{
  if (mCursor >= mLength)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Nibbler::dump ()
{
  return std::string ("Nibbler ‹")
         + mInput.substr (mCursor)
         + "›";
}

////////////////////////////////////////////////////////////////////////////////
