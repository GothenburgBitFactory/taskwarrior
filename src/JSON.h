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

#ifndef INCLUDED_JSON
#define INCLUDED_JSON

#include <map>
#include <vector>
#include <string>
#include <Nibbler.h>

namespace json
{
  enum jtype
  {
    j_value,
    j_object,
    j_array,
    j_string,
    j_number,
    j_literal
  };

  class value
  {
  public:
    value () {}
    virtual ~value () {}
    static value* parse (Nibbler&);
    virtual jtype type ();
    virtual std::string dump ();
  };

  class string : public value, public std::string
  {
  public:
    string () {}
    string (const std::string&);
    ~string () {}
    static string* parse (Nibbler&);
    jtype type ();
    std::string dump ();
  };

  class number : public value, public std::string
  {
  public:
    number () : _dvalue (0.0) {}
    ~number () {}
    static number* parse (Nibbler&);
    jtype type ();
    std::string dump ();
    operator double () const;

    double _dvalue;
  };

  class literal : public value
  {
  public:
    literal () : _lvalue (none) {}
    ~literal () {}
    static literal* parse (Nibbler&);
    jtype type ();
    std::string dump ();

    enum literal_value {none, nullvalue, falsevalue, truevalue};
    literal_value _lvalue;
  };

  class array : public value, public std::vector <value*>
  {
  public:
    array () {}
    ~array ();
    static array* parse (Nibbler&);
    jtype type ();
    std::string dump ();
  };

  class object : public value, public std::map <std::string, value*>
  {
  public:
    object () {}
    ~object ();
    static object* parse (Nibbler&);
    static bool parse_pair (Nibbler&, std::string&, value*&);
    jtype type ();
    std::string dump ();
  };

  // Parser entry point.
  value* parse (const std::string&);

  // Encode/decode for JSON entities.
  std::string encode (const std::string&);
  std::string decode (const std::string&);
}

#endif
////////////////////////////////////////////////////////////////////////////////
