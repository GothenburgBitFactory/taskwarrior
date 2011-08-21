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
#ifndef INCLUDED_ARG
#define INCLUDED_ARG
#define L10N                                           // Localization complete.

#include <string>

#define ARGUMENTS_SEQUENCE_MAX_RANGE 1000

class Arg
{
public:
  enum category {cat_none=1, cat_terminator, cat_program, cat_command, cat_rc, cat_override, cat_attr, cat_attmod, cat_id, cat_uuid, cat_subst, cat_pattern, cat_rx, cat_tag, cat_dom, cat_op, cat_literal};
  enum type     {type_none=20, type_pseudo, type_bool, type_string, type_date, type_duration, type_number};

  Arg ();
  Arg (const std::string&);
  Arg (const std::string&, const category);
  Arg (const std::string&, const type, const category);
  Arg (const Arg&);
  Arg& operator= (const Arg&);
  bool operator== (const Arg&) const;

  static const type type_id (const std::string&);
  static const std::string type_name (type);

  static const category category_id (const std::string&);
  static const std::string category_name (category);

public:
  std::string _value;    // Interpreted value
  std::string _raw;      // Raw input token, never modified
  type        _type;     // Data type
  category    _category; // Categorized argument
};

#endif
////////////////////////////////////////////////////////////////////////////////
