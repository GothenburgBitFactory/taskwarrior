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
#ifndef INCLUDED_ARGUMENTS
#define INCLUDED_ARGUMENTS
#define L10N                                           // Localization complete.

#include <vector>
#include <string>
#include <File.h>

#define ARGUMENTS_SEQUENCE_MAX_RANGE 1000

class Triple
{
public:
  Triple (
    const std::string& one,
    const std::string& two,
    const std::string& three)
  : _first (one)
  , _second (two)
  , _third (three)
  {
  }

  Triple (const Triple& other)
  {
    _first  = other._first;
    _second = other._second;
    _third  = other._third;
  }

  Triple& operator= (const Triple& other)
  {
    if (this != &other)
    {
      _first  = other._first;
      _second = other._second;
      _third  = other._third;
    }

    return *this;
  }

  bool operator== (const Triple& other) const
  {
    return _first  == other._first  &&
           _second == other._second &&
           _third  == other._third;
  }

public:
  std::string _first;
  std::string _second;
  std::string _third;
};

//class Arguments : public std::vector <std::pair <std::string, std::string> >
class Arguments : public std::vector <Triple>
{
public:
  Arguments ();
  ~Arguments ();

  void capture (int, const char**);
  void capture (const std::string&);
  void capture_first (const std::string&);
  void categorize ();

  void append_stdin ();
  void rc_override (std::string&, File&);
  void get_data_location (std::string&);
  void apply_overrides ();
  void resolve_aliases ();
  void inject_defaults ();

  std::vector <std::string> list ();
  static std::vector <std::string> operator_list ();
  std::string combine ();

  bool find_command (std::string&);
  std::string find_limit ();

  static bool is_multipart (const std::string&, std::vector <std::string>&);
  static bool is_command (const std::vector <std::string>&, std::string&);
  static bool is_attr (const std::string&);
  static bool is_attmod (const std::string&);
  static bool is_subst (const std::string&);
  static bool is_pattern (const std::string&);
  static bool is_id (const std::string&);
  static bool is_uuid (const std::string&);
  static bool is_tag (const std::string&);
  static bool is_operator (const std::string&);
  static bool is_operator (const std::string&, char&, int&, char&);
  static bool is_symbol_operator (const std::string&);
  static bool is_attribute (const std::string&, std::string&);
  static bool is_modifier (const std::string&);
  static bool is_expression (const std::string&);

  static bool extract_attr (const std::string&, std::string&, std::string&);
  static bool extract_attmod (const std::string&, std::string&, std::string&, std::string&, std::string&);
  static bool extract_subst (const std::string&, std::string&, std::string&, bool&);
  static bool extract_pattern (const std::string&, std::string&);
  static bool extract_id (const std::string&, std::vector <int>&);
  static bool extract_uuid (const std::string&, std::vector <std::string>&);
  static bool extract_tag (const std::string&, char&, std::string&);
  static bool extract_operator (const std::string&, std::string&);

  Arguments extract_read_only_filter ();
  Arguments extract_write_filter ();
  Arguments extract_modifications ();

  static bool valid_modifier (const std::string&);

  void dump (const std::string&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
