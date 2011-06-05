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

class Arguments : public std::vector <std::pair <std::string, std::string> >
{
public:
  Arguments ();
  ~Arguments ();

  void capture (int, const char**);
  void capture (const std::string&);
  void categorize ();

  void append_stdin ();
  void rc_override (std::string&, File&);
  void get_data_location (std::string&);
  void apply_overrides ();
  void resolve_aliases ();

  std::vector <std::string> list ();
  std::string combine ();

  bool find_command (std::string&);

  bool is_command (const std::vector <std::string>&, std::string&);
  bool is_attr (const std::string&);
  bool is_attmod (const std::string&);
  bool is_subst (const std::string&);
  bool is_pattern (const std::string&);
  bool is_id (const std::string&);
  bool is_uuid (const std::string&);
  bool is_tag (const std::string&);
  bool is_operator (const std::string&);

  // TODO Decide if these are really useful.
  bool extract_attr (const std::string&, std::string&, std::string&);
  bool extract_attmod (const std::string&, std::string&, std::string&, std::string&, std::string&);
  bool extract_subst (const std::string&, std::string&, std::string&, bool&);
  bool extract_pattern (const std::string&, std::string&);
  bool extract_id (const std::string&, std::vector <int>&);
  bool extract_uuid (const std::string&, std::vector <std::string>&);
  bool extract_tag (const std::string&, char&, std::string&);
  bool extract_operator (const std::string&, std::string&);

  Arguments extract_read_only_filter ();
  Arguments extract_write_filter ();
  Arguments extract_modifications ();

  bool valid_modifier (const std::string&);

  void dump (const std::string&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
