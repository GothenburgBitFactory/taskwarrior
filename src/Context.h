////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include "Filter.h"
#include "Keymap.h"
#include "Config.h"
#include "Sequence.h"
#include "Subst.h"
#include "Cmd.h"
#include "Task.h"
#include "TDB.h"
#include "StringTable.h"

class Context
{
public:
  Context ();                          // Default constructor
  ~Context ();                         // Destructor

  Context (const Context&);
  Context& operator= (const Context&);

  void initialize (int, char**);       // all startup
  void initialize ();                  // for reinitializing
  int run ();                          // task classic
  int interactive ();                  // task interactive (not implemented)
  std::string dispatch ();             // command handler dispatch
  void shadow ();                      // shadow file update

  int getWidth ();                     // determine terminal width

  void header (const std::string&);    // Header message sink
  void footnote (const std::string&);  // Footnote message sink
  void debug (const std::string&);     // Debug message sink
  void clearMessages ();

  void parse ();
  void parse (std::vector <std::string>&, Cmd&, Task&, Sequence&, Subst&, Filter&);
  void clear ();

  std::string canonicalize (const std::string&) const;

private:
  void loadCorrectConfigFile ();
  void loadAliases ();
  void autoFilter (Task&, Filter&);

public:
  Config                    config;
  Filter                    filter;
  Keymap                    keymap;
  Sequence                  sequence;
  Subst                     subst;
  Task                      task;
  TDB                       tdb;
  StringTable               stringtable;
  std::string               program;
  std::vector <std::string> args;
  Cmd                       cmd;
  std::map <std::string, std::string> aliases;
  std::vector <std::string> tagAdditions;
  std::vector <std::string> tagRemovals;

private:
  std::vector <std::string> headers;
  std::vector <std::string> footnotes;
  std::vector <std::string> debugMessages;
  bool                      inShadow;
};

#endif
////////////////////////////////////////////////////////////////////////////////
