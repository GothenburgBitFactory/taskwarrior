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

  void initialize (int, char**);       // all startup
  void initialize ();                  // for reinitializing
  int run ();                          // task classic
  int interactive ();                  // task interactive (not implemented)
  std::string dispatch ();             // command handler dispatch
  void shadow ();                      // shadow file update

  int getWidth ();                     // determine terminal width

  void message (const std::string&);   // Message sink
  void footnote (const std::string&);  // Footnote sink

private:
  void loadCorrectConfigFile ();
  void parse ();
  void constructFilter ();

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
  std::vector <std::string> tagAdditions;  // TODO This is redundant, remove.
  std::vector <std::string> tagRemovals;

private:
  std::vector <std::string> messages;
  std::vector <std::string> footnotes;
  bool                      inShadow;
};

#endif
////////////////////////////////////////////////////////////////////////////////
