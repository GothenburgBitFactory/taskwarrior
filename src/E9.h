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
#ifndef INCLUDED_E9
#define INCLUDED_E9
#define L10N                                           // Localization complete.

#include <string>
#include <A3.h>
#include <Task.h>
#include <RX.h>

class Term
{
public:
  Term ()
  : _raw ("")
  , _value ("")
  , _category ("")
  {
  }

  Term (const Arg& arg)
  : _raw (arg._raw)
  , _value (arg._raw)
  , _category (arg._category)
  {
  }

  Term (
    const std::string& raw,
    const std::string& value,
    const std::string& category)
  : _raw (raw)
  , _value (value)
  , _category (category)
  {
  }

  Term (const Term& other)
  {
    _raw      = other._raw;
    _value    = other._value;
    _category = other._category;
  }

  Term& operator= (const Term& other)
  {
    if (this != &other)
    {
      _raw      = other._raw;
      _value    = other._value;
      _category = other._category;
    }

    return *this;
  }

  bool operator== (const Term& other) const
  {
    return _raw      == other._raw   &&
           _value    == other._value &&
           _category == other._category;
  }

public:
  std::string _raw;      // Raw input token, never modified
  std::string _value;    // Evaluated raw token, sometimes identical
  std::string _category; // Categorized argument
};

class E9
{
public:
  E9 (const A3&);
  ~E9 ();

  bool evalFilter (const Task&);
  std::string evalExpression (const Task&);

private:
  void eval (const Task&, std::vector <Term>&);
  bool eval_match (Term&, Term&, bool);

  // Unary.
  void operator_not (Term&, Term&);

  // Binary.
  void operator_and      (Term&, Term&, Term&);
  void operator_or       (Term&, Term&, Term&);
  void operator_xor      (Term&, Term&, Term&);
  void operator_lt       (Term&, Term&, Term&);
  void operator_lte      (Term&, Term&, Term&);
  void operator_gte      (Term&, Term&, Term&);
  void operator_gt       (Term&, Term&, Term&);
  void operator_inequal  (Term&, Term&, Term&, bool);
  void operator_equal    (Term&, Term&, Term&, bool);
  void operator_match    (Term&, Term&, Term&, bool);
  void operator_nomatch  (Term&, Term&, Term&, bool);
  void operator_multiply (Term&, Term&, Term&);
  void operator_divide   (Term&, Term&, Term&);
  void operator_add      (Term&, Term&, Term&);
  void operator_subtract (Term&, Term&, Term&);

  const Term coerce (const Term&, const std::string&);
  bool get_bool (const Term&);

private:
  std::vector <Term> _terms;
  std::map <std::string, RX> _regexes;
};

#endif
////////////////////////////////////////////////////////////////////////////////
