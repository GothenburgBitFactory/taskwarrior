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
#ifndef INCLUDED_CONTEXT
#define INCLUDED_CONTEXT
#define L10N                                           // Localization complete.

#include <Command.h>
#include <Column.h>
#include <Config.h>
#include <Task.h>
#include <TDB.h>
#include <TDB2.h>
#include <Hooks.h>
#include <DOM.h>
#include <Path.h>
#include <File.h>
#include <Directory.h>
#include <Arguments.h>

class Context
{
public:
  Context ();                          // Default constructor
  ~Context ();                         // Destructor

  Context (const Context&);
  Context& operator= (const Context&);

  void initialize (int, const char**); // all startup
  int run ();
  int dispatch (std::string&);         // command handler dispatch
  void shadow ();                      // shadow file update

  int getWidth ();                     // determine terminal width
  int getHeight ();                    // determine terminal height

  bool color ();                       // TTY or <other>?
  bool verbose (const std::string&);   // Verbosity control

  void header (const std::string&);    // Header message sink
  void footnote (const std::string&);  // Footnote message sink
  void debug (const std::string&);     // Debug message sink
  void clearMessages ();
  void clear ();

  void disallowModification () const;
  void decomposeSortField (const std::string&, std::string&, bool&);

private:
  void assumeLocations ();
  void determineDataLocation ();
  void createDefaultConfig ();
  void loadAliases ();
  void updateXtermTitle ();

public:
  std::string                         program;
  Arguments                           args;
  std::string                         home_dir;
  File                                rc_file;
  Path                                data_dir;
  Directory                           extension_dir;
  Config                              config;

  TDB                                 tdb;                // TODO Obsolete
  TDB2                                tdb2;
  std::map <std::string, std::string> aliases;
  Hooks                               hooks;
  DOM                                 dom;

  // Color
  bool                                determine_color_use;
  bool                                use_color;

  bool                                verbosity_legacy;
  std::vector <std::string>           verbosity;
  std::vector <std::string>           headers;
  std::vector <std::string>           footnotes;
  std::vector <std::string>           debugMessages;
/*
  bool                                inShadow;
*/

  std::map <std::string, Command*>    commands;

  int                                 terminal_width;
  int                                 terminal_height;
};

#endif
////////////////////////////////////////////////////////////////////////////////
