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
#define L10N                                           // Localization complete.

#include <map>
#include <vector>
#include <string>
#include <Nibbler.h>

namespace json
{
  enum jtype
  {
    j_value,    // 0
    j_object,   // 1
    j_array,    // 2
    j_string,   // 3
    j_number,   // 4
    j_literal   // 5
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

  class string : public value
  {
  public:
    string () {}
    string (const std::string&);
    ~string () {}
    static string* parse (Nibbler&);
    jtype type ();
    std::string dump ();

  public:
    std::string _data;
  };

  class number : public value
  {
  public:
    number () : _dvalue (0.0) {}
    ~number () {}
    static number* parse (Nibbler&);
    jtype type ();
    std::string dump ();
    operator double () const;

  public:
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

  public:
    enum literal_value {none, nullvalue, falsevalue, truevalue};
    literal_value _lvalue;
  };

  class array : public value
  {
  public:
    array () {}
    ~array ();
    static array* parse (Nibbler&);
    jtype type ();
    std::string dump ();

  public:
    std::vector <value*> _data;
  };

  class object : public value
  {
  public:
    object () {}
    ~object ();
    static object* parse (Nibbler&);
    static bool parse_pair (Nibbler&, std::string&, value*&);
    jtype type ();
    std::string dump ();

  public:
    std::map <std::string, value*> _data;
  };

  // Parser entry point.
  value* parse (const std::string&);

  // Encode/decode for JSON entities.
  std::string encode (const std::string&);
  std::string decode (const std::string&);
}

typedef std::vector <json::value*>::iterator           json_array_iter;
typedef std::map <std::string, json::value*>::iterator json_object_iter;

#endif
////////////////////////////////////////////////////////////////////////////////
