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
  UnitTest t (9);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  Task::attributes["description"] = "string";
  Task::attributes["entry"] = "date";
  Task::attributes["tags"] = "string";
  Task::attributes["uuid"] = "string";

  bool good = true;
  try {Task t1 ("{}");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('{}')");

  good = true;
  try {Task t2 ("{\"uuid\":\"00000000-0000-0000-000000000001\",\"description\":\"foo\",\"entry\":\"1234567890\"}");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('{<minimal>}')");

  // Verify tag handling is correct between F4 and JSON.
  Task t3;
  t3.set ("entry", "20130602T224000Z");
  t3.set ("description", "DESC");
  t3.addTag ("tag1");
  t.is (t3.composeF4 (), "[description:\"DESC\" entry:\"20130602T224000Z\" tags:\"tag1\"]", "F4 good");
  t.is (t3.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\"]}", "JSON good");

  t3.addTag ("tag2");
  t.is (t3.composeF4 (), "[description:\"DESC\" entry:\"20130602T224000Z\" tags:\"tag1,tag2\"]", "F4 good");
  t.is (t3.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}", "JSON good");

  good = true;
  Task t4;
  try {t4 = Task ("{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}");}
  catch (const std::string& e){t.diag (e); good = false;}
  t.ok (good, "Task::Task ('{two tags}')");
  t.is (t4.composeF4 (), "[description:\"DESC\" entry:\"1370212800\" tags:\"tag1,tag2\"]", "F4 good");
  t.is (t4.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}", "JSON good");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
