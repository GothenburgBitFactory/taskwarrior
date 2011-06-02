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

class Arguments : public std::vector <std::string>
{
public:
  Arguments ();
  ~Arguments ();

  void capture (int, const char**);
  void append_stdin ();
  void rc_override (std::string&, File&, std::string&);
  void get_data_location (std::string&);
  void apply_overrides (std::string&);
  void resolve_aliases ();
  std::string combine ();

  bool extract_command (const std::vector <std::string>&, std::string&);
  void extract_filter ();
  void extract_modifications ();

  void extract_sequence (std::vector <int>&);
  void extract_uuids (std::vector <std::string>&);
  void extract_attrs ();
  void extract_words ();
  void extract_tags ();
  void extract_pattern ();
  void extract_subst ();
};

#endif
////////////////////////////////////////////////////////////////////////////////
