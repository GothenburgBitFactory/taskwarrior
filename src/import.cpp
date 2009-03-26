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
#include <iostream>
#include <sstream>
#include "task.h"

////////////////////////////////////////////////////////////////////////////////
enum fileType
{
  not_a_clue,
  task_1_4_3,
  task_1_5_0,
  task_cmd_line,
  todo_sh_2_0,
  csv,
  text
};

static fileType determineFileType (const std::vector <std::string>& lines)
{
  // '7f7a4191-c2f2-487f-8855-7a1eb378c267',' ...
  // ....:....|....:....|....:....|....:....|
  // 1        10        20        30        40
  if (lines.size () > 1    &&
      lines[1][0]  == '\'' &&
      lines[1][9]  == '-'  &&
      lines[1][14] == '-'  &&
      lines[1][19] == '-'  &&
      lines[1][24] == '-'  &&
      lines[1][37] == '\'' &&
      lines[1][38] == ','  &&
      lines[1][39] == '\'')
  {
    if (lines[0] == "'id','uuid','status','tags','entry','start','due','recur',"
                    "'end','project','priority','fg','bg','description'")
      return task_1_5_0;

    if (lines[0] == "'id','status','tags','entry','start','due','end','project',"
                    "'priority','fg','bg','description'")
      return task_1_4_3;
  }

  // TODO task_cmd_line

  // x 2009-03-25 Walk the dog +project @context
  // This is a test +project @context
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    // All done tasks begin with "x YYYY-MM-DD".
    if (lines[i].length () > 12)
    {
      if (           lines[i][0] == 'x' &&
                     lines[i][1] == ' ' &&
          ::isdigit (lines[i][2]) &&
          ::isdigit (lines[i][3]) &&
          ::isdigit (lines[i][4]) &&
          ::isdigit (lines[i][5]) &&
                     lines[i][6] == '-' &&
          ::isdigit (lines[i][7]) &&
          ::isdigit (lines[i][8]) &&
                     lines[i][9] == '-' &&
          ::isdigit (lines[i][10]) &&
          ::isdigit (lines[i][11]))
        return todo_sh_2_0;
    }

    std::vector <std::string> words;
    split (words, lines[i], ' ');
    for (unsigned int w = 0; w < words.size (); ++w)
    {
      // +project
      if (words[w].length () > 1 &&
          words[w][0] == '+'     &&
          ::isalnum (words[w][1]))
        return todo_sh_2_0;

      // @context
      if (words[w].length () > 1 &&
          words[w][0] == '@'     &&
          ::isalnum (words[w][1]))
        return todo_sh_2_0;
    }
  }

  // CSV - commas on every non-comment, non-trivial line.
  bool commas_on_every_line = true;
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    if (lines[i].length () > 10 &&
        lines[i][0] != '#' &&
        lines[i].find (",") == std::string::npos)
    {
      commas_on_every_line = false;
      break;
    }
  }
  if (commas_on_every_line)
    return csv;

  // TODO text, possibly with commas.
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    // TODO priority:{H,M,L}
    // TODO priorit:{H,M,L}
    // TODO priori:{H,M,L}
    // TODO prior:{H,M,L}
    // TODO prio:{H,M,L}
    // TODO pri:{H,M,L}
    // TODO project:.+
    // TODO projec:.+
    // TODO proje:.+
    // TODO proj:.+
    // TODO pro:.+
  }

  return not_a_clue;
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTask_1_4_3 (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  return "task 1.4.3\n";
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTask_1_5_0 (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  return "task 1.5.0\n";
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTaskCmdLine (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  return "task command line\n";
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTodoSh_2_0 (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  return "todo.sh 2.0\n";
}

////////////////////////////////////////////////////////////////////////////////
static std::string importText (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    std::string line = lines[i];

    // Strip comments
    std::string::size_type pound = line.find ("#");
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    // Skip blank lines
    if (line.length () > 0)
    {
      T task;
      task.parse (std::string ("\"") + lines[i] + "\"");
      if (! tdb.addT (task))
        failed.push_back (lines[i]);
    }
  }

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size ())
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  std::string bad;
  join (bad, "\n", failed);
  return out.str () + "\n" + bad;
}

////////////////////////////////////////////////////////////////////////////////
static std::string importCSV (
  TDB& tdb,
  Config& conf,
  const std::vector <std::string>& lines)
{
  // TODO Allow any number of fields, but attempt to map into task fields.
  // TODO Must have header line to name fields.
  return "CSV\n";
}

////////////////////////////////////////////////////////////////////////////////
std::string handleImport (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Use the description as a file name.
  std::string file = trim (task.getDescription ());
  if (file.length () > 0)
  {
    // Load the file.
    std::vector <std::string> lines;
    slurp (file, lines, true);

    // Determine which type it might be, then attempt an import.
    switch (determineFileType (lines))
    {
    case task_1_4_3:    out << importTask_1_4_3  (tdb, conf, lines); break;
    case task_1_5_0:    out << importTask_1_5_0  (tdb, conf, lines); break;
    case task_cmd_line: out << importTaskCmdLine (tdb, conf, lines); break;
    case todo_sh_2_0:   out << importTodoSh_2_0  (tdb, conf, lines); break;
    case csv:           out << importCSV         (tdb, conf, lines); break;
    case text:          out << importText        (tdb, conf, lines); break;

    case not_a_clue:
      out << "?";
      break;
    }
  }
  else
    throw std::string ("You must specify a file to import.");

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////

