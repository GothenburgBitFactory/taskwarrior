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

#include <cmake.h>
#include <iostream>
#include <stdlib.h>
#include <Context.h>
#include <Task.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (18);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  // (blank)
  bool good = true;
  Task task;

  try {task = Task ("");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.notok (good, "Task::Task ('')");

  // []
  good = true;
  try {task = Task ("[]");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.notok (good, "Task::Task ('[]')");

  // [name:"value"]
  good = true;
  try {task = Task ("[name:\"value\"]");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[name:\"value\"]')");
  t.is (task.get ("name"), "value", "name=value");

  // [name:"one two"]
  good = true;
  try {task = Task ("[name:\"one two\"]");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[name:\"one two\"]')");
  t.is (task.get ("name"), "one two", "name=one two");

  // [one:two three:four]
  good = true;
  try {task = Task ("[one:\"two\" three:\"four\"]");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('[one:\"two\" three:\"four\"]')");
  t.is (task.get ("one"), "two", "one=two");
  t.is (task.get ("three"), "four", "three=four");

  // Task::set
  task.clear ();
  task.set ("name", "value");
  t.is (task.composeF4 (), "[name:\"value\"]", "Task::set");

  // Task::has
  t.ok    (task.has ("name"), "Task::has");
  t.notok (task.has ("woof"), "Task::has not");

  // Task::get_int
  task.set ("one", 1);
  t.is (task.composeF4 (), "[name:\"value\" one:\"1\"]", "Task::set");
  t.is (task.get_int ("one"), 1, "Task::get_int");

  // Task::get_ulong
  task.set ("two", "4294967295");
  t.is (task.composeF4 (), "[name:\"value\" one:\"1\" two:\"4294967295\"]", "Task::set");
  t.is ((size_t)task.get_ulong ("two"), (size_t)4294967295UL, "Task::get_ulong");

  // Task::remove
  task.remove ("one");
  task.remove ("two");
  t.is (task.composeF4 (), "[name:\"value\"]", "Task::remove");

  // Task::all
  t.is (task.size (), (size_t)1, "Task::all size");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
