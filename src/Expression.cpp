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

#include <Context.h>
#include <Expression.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Expression::Expression (Arguments& arguments)
: _original (arguments)
{
}

////////////////////////////////////////////////////////////////////////////////
Expression::~Expression ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Expression::eval (Task& task)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Inserts the 'and' operator by default between terms that are not separated by
// at least one operator.
//
// Converts:  <term1>     <term2> <op> <term3>
// to:        <term1> and <term2> <op> <term3>
//
void Expression::toInfix ()
{
  _infix.clear ();

  std::string previous = "op";
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = _original.begin (); arg != _original.end (); ++arg)
  {
    if (previous    != "op" &&
        arg->second != "op")
      _infix.push_back (std::make_pair ("and", "op"));

    _infix.push_back (*arg);
    previous = arg->second;
  }

  _infix.dump ("Expression::toInfix");
}

////////////////////////////////////////////////////////////////////////////////
// Dijkstra Shunting Algorithm.
void Expression::toPostfix ()
{
  _postfix.clear ();

  _postfix.dump ("Expression::toPostfix");
}

////////////////////////////////////////////////////////////////////////////////
void Expression::dump (const std::string& label)
{
}

////////////////////////////////////////////////////////////////////////////////
