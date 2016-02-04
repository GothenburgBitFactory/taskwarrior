////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_VARIANT
#define INCLUDED_VARIANT

#include <map>
#include <string>
#include <time.h>
#include <Task.h>

class Variant
{
public:
  static std::string dateFormat;
  static bool searchCaseSensitive;
  static bool searchUsingRegex;
  static bool isoEnabled;

  enum type {type_boolean, type_integer, type_real, type_string, type_date, type_duration};

  Variant () = default;
  Variant (const Variant&);
  Variant (const bool);
  Variant (const int);
  Variant (const double);
  Variant (const std::string&);
  Variant (const char*);
  Variant (const time_t, const enum type);

  void source (const std::string&);
  const std::string& source () const;

  Variant& operator= (const Variant&);

  bool operator&& (const Variant&) const;
  bool operator|| (const Variant&) const;
  bool operator_xor (const Variant&) const;
  bool operator< (const Variant&) const;
  bool operator<= (const Variant&) const;
  bool operator> (const Variant&) const;
  bool operator>= (const Variant&) const;
  bool operator== (const Variant&) const;
  bool operator!= (const Variant&) const;
  bool operator_match (const Variant&, const Task&) const;
  bool operator_nomatch (const Variant&, const Task&) const;
  bool operator_partial (const Variant&) const;
  bool operator_nopartial (const Variant&) const;
  bool operator_hastag (const Variant&, const Task&) const;
  bool operator_notag (const Variant&, const Task&) const;
  bool operator! () const;

  Variant& operator^= (const Variant&);
  Variant operator^ (const Variant&) const;

  Variant& operator-= (const Variant&);
  Variant operator-   (const Variant&) const;

  Variant& operator+= (const Variant&);
  Variant operator+   (const Variant&) const;

  Variant& operator*= (const Variant&);
  Variant operator*   (const Variant&) const;

  Variant& operator/= (const Variant&);
  Variant operator/   (const Variant&) const;

  Variant& operator%= (const Variant&);
  Variant operator%   (const Variant&) const;

  operator std::string () const;
  void sqrt ();

  void cast (const enum type);
  int type ();
  bool trivial () const;

  bool               get_bool () const;
  int                get_integer () const;
  double             get_real () const;
  const std::string& get_string () const;
  time_t             get_date () const;
  time_t             get_duration () const;

private:
  enum type   _type     {type_boolean};
  bool        _bool     {false};
  int         _integer  {0};
  double      _real     {0.0};
  std::string _string   {""};
  time_t      _date     {0};
  time_t      _duration {0};

  std::string _source   {""};
};

#endif

////////////////////////////////////////////////////////////////////////////////
