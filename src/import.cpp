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
std::string handleImport (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Use the description as a file name.
  std::string file = trim (task.getDescription ());

  if (file.length () > 0)
  {
    out << "Not yet implemented." << std::endl;
  }
  else
    throw std::string ("You must specify a file to import.");

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// todo.sh v2.x
//         file format: (A) Walk the dog +project @context
//                      x 2009-03-25 Walk the dog +project @context
//            priority: (A) - (Z)
//   multiple projects: +project
//   multiple contexts: @context

// task >= 1.5 export
//         file format: id,uuid,status,tags,entry,start,due,recur,end,project,
//                      priority,fg,bg,description\n

// task < 1.5 export
//         file format: id,uuid,status,tags,entry,start,due,project,priority,
//                      fg,bg,description\n

// single line text
//         file format: foo bar baz

// CSV
//         file format: project,priority,description

////////////////////////////////////////////////////////////////////////////////
void determineFileType ()
{
}

////////////////////////////////////////////////////////////////////////////////
void importTask_1_5 ()
{
}

void importTask_1_6 ()
{
}

void importTodoSh_2_0 ()
{
}

void importSingleLine ()
{
}

void importCSV ()
{
}

////////////////////////////////////////////////////////////////////////////////

