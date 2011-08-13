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
#ifndef INCLUDED_A3
#define INCLUDED_A3
#define L10N                                           // Localization complete.

#include <vector>
#include <string>
#include <Nibbler.h>
#include <File.h>

#define ARGUMENTS_SEQUENCE_MAX_RANGE 1000

class Arg
{
public:
  Arg ()
  : _raw ("")
  , _category ("")
  {
  }

  Arg (
    const std::string& raw,
    const std::string& category)
  : _raw (raw)
  , _category (category)
  {
  }

  Arg (const Arg& other)
  {
    _raw      = other._raw;
    _category = other._category;
  }

  Arg& operator= (const Arg& other)
  {
    if (this != &other)
    {
      _raw      = other._raw;
      _category = other._category;
    }

    return *this;
  }

  bool operator== (const Arg& other) const
  {
    return _raw      == other._raw  &&
           _category == other._category;
  }

public:
  std::string _raw;      // Raw input token, never modified
  std::string _category; // Categorized argument
};

class A3 : public std::vector <Arg>
{
public:
  A3 ();
  A3 (const A3&);
  A3& operator= (const A3&);
  ~A3 ();


  void capture (int, const char**);
  void capture (const std::string&);
  void capture_first (const std::string&);

  void categorize ();
  static bool is_command (const std::vector <std::string>&, std::string&);
  static const std::vector <std::string> operator_list ();

  void append_stdin ();
  void rc_override (std::string&, File&);
  void get_data_location (std::string&);
  void resolve_aliases ();
  void apply_overrides ();
  void inject_defaults ();

  const std::string combine () const;
  const std::vector <std::string> list () const;
  bool find_command (std::string&) const;
  const std::string find_limit () const;

  const A3 extract_filter () const;
  const A3 extract_modifications () const;
  const std::vector <std::string> extract_words () const;

  const A3 tokenize (const A3&) const;
  const A3 infix (const A3&) const;
  const A3 expand (const A3&) const;
  const A3 sequence (const A3&) const;
  const A3 postfix (const A3&) const;

  static bool is_attr (Nibbler&, std::string&);
  static bool is_attmod (Nibbler&, std::string&);
  static bool is_attribute (const std::string&, std::string&);
  static bool is_modifier (const std::string&, std::string&);
  static bool is_dom (Nibbler&, std::string&);
  static bool is_duration (Nibbler&, std::string&);
  static bool is_pattern (Nibbler&, std::string&);
  static bool is_subst (Nibbler&, std::string&);
  static bool is_id (Nibbler&, std::string&);
  static bool is_uuid (Nibbler&, std::string&);
  static bool is_tag (Nibbler&, std::string&);
  static bool is_number (Nibbler&, double&);
  static bool is_integer (Nibbler&, int&);

  static bool extract_pattern (const std::string&, std::string&);
  static bool extract_tag (const std::string&, char&, std::string&);
  static bool extract_attr (const std::string&, std::string&, std::string&);
  static bool extract_attmod (const std::string&, std::string&, std::string&, std::string&, std::string&);
  static bool extract_subst (const std::string&, std::string&, std::string&, bool&);
  static bool extract_id (const std::string&, std::vector <int>&);
  static bool extract_uuid (const std::string&, std::vector <std::string>&);

  static bool is_operator (const std::string&, char&, int&, char&);

  void dump (const std::string&);

private:
  bool _read_only_command;
  std::string _limit;
};

#endif
////////////////////////////////////////////////////////////////////////////////
