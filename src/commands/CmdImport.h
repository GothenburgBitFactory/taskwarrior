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
#ifndef INCLUDED_CMDIMPORT
#define INCLUDED_CMDIMPORT
#define L10N                                           // Localization complete.

#include <string>
#include <Command.h>

class CmdImport : public Command
{
public:
  CmdImport ();
  int execute (const std::string&, std::string&);

private:
  enum fileType
  {
    type_not_a_clue,
    type_task_1_4_3,
    type_task_1_5_0,
    type_task_1_6_0,
    type_task_cmd_line,
    type_todo_sh_2_0,
    type_csv,
    type_yaml,
    type_text
  };

  fileType determineFileType (const std::vector <std::string>&);
  void decorateTask (Task&);
  std::string task_1_4_3 (const std::vector <std::string>&);
  std::string task_1_5_0 (const std::vector <std::string>&);
  std::string task_1_6_0 (const std::vector <std::string>&);
  std::string taskCmdLine (const std::vector <std::string>&);
  std::string todoSh_2_0 (const std::vector <std::string>&);
  std::string text (const std::vector <std::string>&);
  std::string CSV (const std::vector <std::string>&);
  std::string YAML (const std::vector <std::string>&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
