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
#include <iostream>
#include <Context.h>
#include <Att.h>
#include <Task.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (18);

  // (blank)
  bool good = true;
  Task task;

  try {task = Task ("");}
  catch (std::string& e){t.diag (e); good = false;}
  t.notok (good, "Task::Task ('')");

  // []
  good = true;
  try {task = Task ("[]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.notok (good, "Task::Task ('[]')");

  // [name:"value"]
  good = true;
  try {task = Task ("[name:\"value\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[name:\"value\"]')");
  t.is (task.get ("name"), "value", "name=value");

  // [name:"one two"]
  good = true;
  try {task = Task ("[name:\"one two\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[name:\"one two\"]')");
  t.is (task.get ("name"), "one two", "name=one two");

  // [one:two three:four]
  good = true;
  try {task = Task ("[one:\"two\" three:\"four\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[one:\"two\" three:\"four\"]')");
  t.is (task.get ("one"), "two", "one=two");
  t.is (task.get ("three"), "four", "three=four");

  // Task::set
  task.clear ();
  task.set ("name", "value");
  t.is (task.composeF4 (), "[name:\"value\"]\n", "Task::set");

  // Task::has
  t.ok    (task.has ("name"), "Task::has");
  t.notok (task.has ("woof"), "Task::has not");

  // Task::get_int
  task.set ("one", 1);
  t.is (task.composeF4 (), "[name:\"value\" one:\"1\"]\n", "Task::set");
  t.is (task.get_int ("one"), 1, "Task::get_int");

  // Task::get_ulong
  task.set ("two", "4294967295");
  t.is (task.composeF4 (), "[name:\"value\" one:\"1\" two:\"4294967295\"]\n", "Task::set");
  t.is ((size_t)task.get_ulong ("two"), (size_t)4294967295, "Task::get_ulong");

  // Task::remove
  task.remove ("one");
  task.remove ("two");
  t.is (task.composeF4 (), "[name:\"value\"]\n", "Task::remove");

  // Task::all
  t.is (task.size (), (size_t)1, "Task::all size");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
