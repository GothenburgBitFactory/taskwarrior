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
#ifndef INCLUDED_VARIANT
#define INCLUDED_VARIANT
#define L10N                                           // Localization complete.

#include <string>
#include <time.h>
#include <Date.h>
#include <Duration.h>

class Variant
{
public:
  enum variant_type
  {
    v_unknown  = 1,
    v_boolean  = 2,
    v_integer  = 4,
    v_double   = 8,
    v_string   = 16,
    v_date     = 32,
    v_duration = 64,
    v_other    = 128
  };

  Variant ();
  Variant (const Variant&);
  Variant (const bool);
  Variant (const int);
  Variant (const double&);
  Variant (const std::string&);
  Variant (const Date&);
  Variant (const Duration&);

  Variant& operator&& (const Variant& other);
  Variant& operator|| (const Variant& other);
  Variant& operator<= (const Variant& other);
  Variant& operator>= (const Variant& other);
  Variant& operator== (const Variant& other);
  Variant& operator!= (const Variant& other);
  Variant& operator^ (const Variant& other);
  Variant& operator! ();
  Variant& operator- (const Variant& other);
  Variant& operator+ (const Variant& other);
  Variant& operator* (const Variant& other);
  Variant& operator/ (const Variant& other);
  Variant& operator< (const Variant& other);
  Variant& operator> (const Variant& other);

  void sqrt  ();
  void sin   ();
  void cos   ();
  void tan   ();
  void asin  ();
  void acos  ();
  void atan  ();
  void log   ();
  void exp   ();
  void exp10 ();
  void ln    ();
  void sign  ();
  void abs   ();

  void input (const std::string&);
  std::string format ();
  void cast (const variant_type);
  variant_type type ();
  void promote (Variant&, Variant&);

private:
  variant_type mType;

  bool mBool;
  int mInteger;
  double mDouble;
  std::string mString;
  Date mDate;
  Duration mDuration;
};

#endif

////////////////////////////////////////////////////////////////////////////////
