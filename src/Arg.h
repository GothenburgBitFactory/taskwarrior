////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
