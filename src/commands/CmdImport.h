////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CMDIMPORT
#define INCLUDED_CMDIMPORT

#include <string>
#include <Command.h>

class CmdImport : public Command
{
public:
  CmdImport ();
  int execute (std::string&);
  int import (std::vector <std::string>& lines);
};

/*
class CmdImport : public Command
{
public:
  CmdImport ();
  int execute (std::string&);

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
*/

#endif
////////////////////////////////////////////////////////////////////////////////
