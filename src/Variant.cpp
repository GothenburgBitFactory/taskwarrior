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
#include <iostream>
#include <math.h>
#include <Text.h>
#include <Variant.h>

////////////////////////////////////////////////////////////////////////////////
Variant::Variant () :
  mType (v_unknown)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Variant& other)
{
  mType = other.mType;

  // Explicitly copy only the relevant type.  This saves memory.
  switch (mType)
  {
  case v_boolean:
    mBool = other.mBool;
    break;

  case v_integer:
    mInteger = other.mInteger;
    break;

  case v_double:
    mDouble = other.mDouble;
    break;

  case v_string:
    mString = other.mString;
    break;

  case v_date:
    mDate = other.mDate;
    break;

  case v_duration:
    mDuration = other.mDuration;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const bool input)
{
  mType = v_boolean;
  mBool = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const int input)
{
  mType = v_integer;
  mInteger = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const double& input)
{
  mType = v_double;
  mDouble = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const std::string& input)
{
  mType = v_string;
  mString = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Date& input)
{
  mType = v_date;
  mDate = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Duration& input)
{
  mType = v_duration;
  mDuration = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator&& (const Variant& other)
{
  cast (v_boolean);
  Variant copy (other);
  copy.cast (v_boolean);
  mBool = (mBool && copy.mBool) ? true : false;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator|| (const Variant& other)
{
  cast (v_boolean);
  Variant copy (other);
  copy.cast (v_boolean);
  mBool = (mBool || copy.mBool) ? true : false;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator<= (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    mBool = false;  // TODO Makes no sense.
    break;

  case v_integer:
    mBool = mInteger <= other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble <= other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating <= 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate <= other.mDate ? true : false;
    break;

  case v_duration:
    mBool = (time_t)mDuration <= (time_t)other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator>= (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    mBool = false;  // TODO Makes no sense.
    break;

  case v_integer:
    mBool = mInteger >= other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble >= other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating >= 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate >= other.mDate ? true : false;
    break;

  case v_duration:
    mBool = (time_t)mDuration >= (time_t)other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator== (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    mBool = mBool == other.mBool ? true : false;
    break;

  case v_integer:
    mBool = mInteger == other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble == other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating == 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate == other.mDate ? true : false;
    break;

  case v_duration:
    mBool = mDuration == other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator!= (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    mBool = mBool != other.mBool ? true : false;
    break;

  case v_integer:
    mBool = mInteger != other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble != other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating != 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate != other.mDate ? true : false;
    break;

  case v_duration:
    mBool = mDuration != other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator^ (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform exponentiation on Boolean types");
    break;

  case v_integer:
    mInteger = (int) pow ((double) mInteger, (double) other.mInteger);
    break;

  case v_double:
    mDouble = pow (mDouble, other.mDouble);
    break;

  case v_string:
    throw std::string ("Cannot perform exponentiation on string types");
    break;

  case v_date:
    throw std::string ("Cannot perform exponentiation on date types");
    break;

  case v_duration:
    throw std::string ("Cannot perform exponentiation on duration types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator! ()
{
  cast (v_boolean);
  mBool = ! mBool;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator- (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform subtraction on Boolean types");
    break;

  case v_integer:
    mInteger -= other.mInteger;
    break;

  case v_double:
    mDouble -= other.mDouble;
    break;

  case v_string:
    throw std::string ("Cannot perform subtraction on string types");
    break;

  case v_date:
    // TODO operator-= only takes integers.
    //mDate -= other.mDate;
    break;

  case v_duration:
    // TODO Missing operator -=
    //mDuration -= other.mDuration;
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator+ (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform addition on Boolean types");
    break;

  case v_integer:
    mInteger += other.mInteger;
    break;

  case v_double:
    mDouble += other.mDouble;
    break;

  case v_string:
    mString += other.mString;
    break;

  case v_date:
    // TODO operator+ only works for int
    //mDate += other.mDate;
    break;

  case v_duration:
    // TODO operator+ missing
    //mDuration += other.mDuration;
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator* (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform multiplication on Boolean types");
    break;

  case v_integer:
    mInteger *= other.mInteger;
    break;

  case v_double:
    mDouble *= other.mDouble;
    break;

  case v_string:
    throw std::string ("Cannot perform multiplication on string types");
    break;

  case v_date:
    throw std::string ("Cannot perform multiplication on date types");
    break;

  case v_duration:
    throw std::string ("Cannot perform multiplication on duration types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator/ (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform division on Boolean types");
    break;

  case v_integer:
    mInteger /= other.mInteger;
    break;

  case v_double:
    mDouble /= other.mDouble;
    break;

  case v_string:
    throw std::string ("Cannot perform division on string types");
    break;

  case v_date:
    throw std::string ("Cannot perform division on date types");
    break;

  case v_duration:
    throw std::string ("Cannot perform division on duration types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator< (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform relationally compare Boolean types");
    break;

  case v_integer:
    mBool = mInteger < other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble < other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating < 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate < other.mDate ? true : false;
    break;

  case v_duration:
    mBool = mDuration < other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator> (const Variant& other)
{
  switch (mType)
  {
  case v_boolean:
    throw std::string ("Cannot perform relationally compare Boolean types");
    break;

  case v_integer:
    mBool = mInteger > other.mInteger ? true : false;
    break;

  case v_double:
    mBool = mDouble > other.mDouble ? true : false;
    break;

  case v_string:
    {
      int collating = strcmp (mString.c_str (), other.mString.c_str ());
      mBool = collating > 0 ? true : false;
    }
    break;

  case v_date:
    mBool = mDate > other.mDate ? true : false;
    break;

  case v_duration:
    mBool = mDuration > other.mDuration ? true : false;
    break;
  }

  mType = v_boolean;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Variant::sqrt ()
{
  cast (v_double);
  if (mDouble < 0.0)
    throw std::string ("Cannot take the square root of a negative number.");
  mDouble = ::sqrt (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::sin ()
{
  cast (v_double);
  mDouble = ::sin (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::cos ()
{
  cast (v_double);
  mDouble = ::cos (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::tan ()
{
  cast (v_double);
  mDouble = ::tan (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::asin ()
{
  cast (v_double);
  mDouble = ::asin (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::acos ()
{
  cast (v_double);
  mDouble = ::acos (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::atan ()
{
  cast (v_double);
  mDouble = ::atan (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::log ()
{
  cast (v_double);
  mDouble = ::log10 (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::exp ()
{
  cast (v_double);
  mDouble = ::exp (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::exp10 ()
{
  cast (v_double);
  mDouble = ::pow (10.0, mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::ln ()
{
  cast (v_double);
  mDouble = ::sqrt (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::sign ()
{
  cast (v_double);
  if (mDouble == 0.0)
    throw std::string ("Divide by zero.");
  mDouble /= fabs (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::abs ()
{
  cast (v_double);
  mDouble = ::fabs (mDouble);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::input (const std::string& input)
{
  if (! compare (input, "true", false) ||
      ! compare (input, "t",    false) ||
      ! compare (input, "yes",  false) ||
      ! compare (input, "on",   false))
  {
    mType = v_boolean;
    mBool = true;
    return;
  }

  if (! compare (input, "false", false) ||
      ! compare (input, "f",     false) ||
      ! compare (input, "no",    false) ||
      ! compare (input, "off",   false))
  {
    mType = v_boolean;
    mBool = false;
    return;
  }

  // TODO Attempt Date (input) parse.
  // TODO Attempt Duration (input) parse.

  bool numeric = true;
  bool period = false;
  for (unsigned int i = 0; i < input.length (); ++i)
  {
    if (input[i] == '.')                        period = true;
    if (!isdigit (input[i]) && input[i] != '.') numeric = false;
  }

  if (numeric)
  {
    if (period)
    {
      mType = v_double;
      mDouble = atof (input.c_str ());
    }
    else
    {
      mType = v_integer;
      mInteger = atoi (input.c_str ());
    }

    return;
  }

  mType = v_string;
  mString = input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Variant::format ()
{
  std::string output;

  switch (mType)
  {
  case v_boolean:
    output = mBool ? "true" : "false";
    break;

  case v_integer:
    {
      char temp [24];
      sprintf (temp, "%d", mInteger);
      output = temp;
    }
    break;

  case v_double:
    {
      char temp [24];
      sprintf (temp, "%g", mDouble);
      output = temp;
    }
    break;

  case v_string:
    output = mString;
    break;

  case v_date:
    // TODO Format mDate.
    break;

  case v_duration:
    // TODO Format mDuration.
    break;

  case v_unknown:
  default:
    throw std::string ("Cannot render an unknown type.");
    break;
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
void Variant::cast (const variant_type type)
{
  if (mType == v_unknown || type == v_unknown)
    throw std::string ("Cannot coerce data either to or from an unknown type");

  // Short circuit.
  if (mType == type)
    return;

  // From v_boolean
  if (mType == v_boolean && type == v_integer)
    mInteger = mBool ? 1 : 0;

  else if (mType == v_boolean && type == v_double)
    mDouble = mBool ? 1.0 : 0.0;

  else if (mType == v_boolean && type == v_string)
    mString = mBool ? "true" : "false";

  // From v_integer
  else if (mType == v_integer && type == v_boolean)
    mBool = mInteger == 0 ? false : true;

  else if (mType == v_integer && type == v_double)
    mDouble = (double)mInteger;

  else if (mType == v_integer && type == v_string)
  {
    char temp [24];
    sprintf (temp, "%d", mInteger);
    mString = temp;
  }

  // From v_double
  else if (mType == v_double && type == v_boolean)
    mBool = mDouble == 0.0 ? false : true;

  else if (mType == v_double && type == v_integer)
    mInteger = (int)mDouble;

  else if (mType == v_double && type == v_string)
  {
    char temp [24];
    sprintf (temp, "%g", mDouble);
    mString = temp;
  }

  // From v_string
  else if (mType == v_string && type == v_boolean)
    mBool = (mString.length () == 0 ||
             mString == "0"         ||
             mString == "0.0") ? false : true;

  else if (mType == v_string && type == v_integer)
    mInteger = atol (mString.c_str ());

  else if (mType == v_string && type == v_double)
    mDouble = atol (mString.c_str ());

  // From v_date


  // From v_duration


  mType = type;
}

////////////////////////////////////////////////////////////////////////////////
Variant::variant_type Variant::type ()
{
  return mType;
}

////////////////////////////////////////////////////////////////////////////////

