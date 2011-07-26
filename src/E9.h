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

class E9
{
public:
  E9 (A3&);
  ~E9 ();

  bool evalFilter (const Task&);
  std::string evalExpression (const Task&);

private:
  void eval (const Task&, std::vector <Arg>&);

  // Unary.
  void operator_not (Arg&, Arg&);

  // Binary.
  void operator_and      (Arg&, Arg&, Arg&);
  void operator_or       (Arg&, Arg&, Arg&);
  void operator_xor      (Arg&, Arg&, Arg&);
  void operator_lt       (Arg&, Arg&, Arg&);
  void operator_lte      (Arg&, Arg&, Arg&);
  void operator_gte      (Arg&, Arg&, Arg&);
  void operator_gt       (Arg&, Arg&, Arg&);
  void operator_inequal  (Arg&, Arg&, Arg&);
  void operator_equal    (Arg&, Arg&, Arg&);
  void operator_match    (Arg&, Arg&, Arg&);
  void operator_nomatch  (Arg&, Arg&, Arg&);
  void operator_multiply (Arg&, Arg&, Arg&);
  void operator_divide   (Arg&, Arg&, Arg&);
  void operator_add      (Arg&, Arg&, Arg&);
  void operator_subtract (Arg&, Arg&, Arg&);

private:
  A3 _args;
  std::map <std::string, RX> _regexes;
};

#endif
////////////////////////////////////////////////////////////////////////////////
