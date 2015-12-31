////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest test (49);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  test.is ((int)Task::textToStatus ("pending"),   (int)Task::pending,   "textToStatus pending");
  test.is ((int)Task::textToStatus ("completed"), (int)Task::completed, "textToStatus completed");
  test.is ((int)Task::textToStatus ("deleted"),   (int)Task::deleted,   "textToStatus deleted");
  test.is ((int)Task::textToStatus ("recurring"), (int)Task::recurring, "textToStatus recurring");

  test.is (Task::statusToText (Task::pending),   "pending",   "statusToText pending");
  test.is (Task::statusToText (Task::completed), "completed", "statusToText completed");
  test.is (Task::statusToText (Task::deleted),   "deleted",   "statusToText deleted");
  test.is (Task::statusToText (Task::recurring), "recurring", "statusToText recurring");

  // Round-trip testing.
  Task t3;
  t3.set ("name", "value");
  std::string before = t3.composeF4 ();
  t3.parse (before);
  std::string after = t3.composeF4 ();
  t3.parse (after);
  after = t3.composeF4 ();
  t3.parse (after);
  after = t3.composeF4 ();
  test.is (before, after, "Task::composeF4 -> parse round trip 4 iterations");

  // Legacy Format 1 (no longer supported)
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  std::string sample = "[tag1 tag2] [att1:value1 att2:value2] Description";
  sample = "X "
           "[one two] "
           "[att1:value1 att2:value2] "
           "Description";
  bool good = true;
  try { Task ff1 (sample); } catch (...) { good = false; }
  test.notok (good, "Support for ff1 removed");

  // Legacy Format 2 (no longer supported)
  //   uuid status [tags] [attributes] description\n
  sample = "00000000-0000-0000-0000-000000000000 "
           "- "
           "[tag1 tag2] "
           "[att1:value1 att2:value2] "
           "Description";
  good = true;
  try { Task ff2 (sample); } catch (...) { good = false; }
  test.notok (good, "Support for ff2 removed");

  // Legacy Format 3
  //   uuid status [tags] [attributes] [annotations] description\n
  sample = "00000000-0000-0000-0000-000000000000 "
           "- "
           "[tag1 tag2] "
           "[att1:value1 att2:value2] "
           "[123:ann1 456:ann2] Description";
  good = true;
  try { Task ff3 (sample); } catch (...) { good = false; }
  test.notok (good, "Support for ff3 removed");

  // Current Format 4
  //   [name:"value" ...]\n
  sample = "["
           "uuid:\"00000000-0000-0000-0000-000000000000\" "
           "status:\"pending\" "
           "tags:\"tag1,tag2\" "
           "att1:\"value1\" "
           "att2:\"value2\" "
           "description:\"Description\""
           "]";
  Task ff4 (sample);
  std::string value = ff4.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "ff4 uuid");
  value = ff4.get ("status");
  test.is (value, "pending", "ff4 status");
  test.ok (ff4.hasTag ("tag1"), "ff4 tag1");
  test.ok (ff4.hasTag ("tag2"), "ff4 tag2");
  test.is (ff4.getTagCount (), 2, "ff4 # tags");
  value = ff4.get ("att1");
  test.is (value, "value1", "ff4 att1");
  value = ff4.get ("att2");
  test.is (value, "value2", "ff4 att2");
  value = ff4.get ("description");
  test.is (value, "Description", "ff4 description");

/*

TODO Task::composeCSV
TODO Task::composeYAML
TODO Task::id
TODO Task::*Status
TODO Task::*Tag*
TODO Task::*Annotation*

TODO Task::addDependency
TODO Task::addDependency
TODO Task::removeDependency
TODO Task::removeDependency
TODO Task::getDependencies
TODO Task::getDependencies

TODO Task::urgency

TODO Task::encode
TODO Task::decode

*/

  // Task::operator==
  Task left ("[one:1 two:2 three:3]");
  Task right (left);
  test.ok (left == right, "left == right -> true");
  left.set ("one", "1.0");
  test.notok (left == right, "left == right -> false");

  ////////////////////////////////////////////////////////////////////////////////
  Task task;

  // (blank)
  good = true;
  try {task = Task ("");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.notok (good, "Task::Task ('')");

  // []
  good = true;
  try {task = Task ("[]");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.notok (good, "Task::Task ('[]')");

  // [name:"value"]
  good = true;
  try {task = Task ("[name:\"value\"]");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('[name:\"value\"]')");
  test.is (task.get ("name"), "value", "name=value");

  // [name:"one two"]
  good = true;
  try {task = Task ("[name:\"one two\"]");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('[name:\"one two\"]')");
  test.is (task.get ("name"), "one two", "name=one two");

  // [one:two three:four]
  good = true;
  try {task = Task ("[one:\"two\" three:\"four\"]");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('[one:\"two\" three:\"four\"]')");
  test.is (task.get ("one"), "two", "one=two");
  test.is (task.get ("three"), "four", "three=four");

  // Task::set
  task.data.clear ();
  task.set ("name", "value");
  test.is (task.composeF4 (), "[name:\"value\"]", "Task::set");

  // Task::has
  test.ok    (task.has ("name"), "Task::has");
  test.notok (task.has ("woof"), "Task::has not");

  // Task::get_int
  task.set ("one", 1);
  test.is (task.composeF4 (), "[name:\"value\" one:\"1\"]", "Task::set");
  test.is (task.get_int ("one"), 1, "Task::get_int");

  // Task::get_ulong
  task.set ("two", "4294967295");
  test.is (task.composeF4 (), "[name:\"value\" one:\"1\" two:\"4294967295\"]", "Task::set");
  test.is ((size_t)task.get_ulong ("two"), (size_t)4294967295UL, "Task::get_ulong");

  // Task::remove
  task.remove ("one");
  task.remove ("two");
  test.is (task.composeF4 (), "[name:\"value\"]", "Task::remove");

  // Task::all
  test.is (task.data.size (), (size_t)1, "Task::all size");

  ////////////////////////////////////////////////////////////////////////////////

  Task::attributes["description"] = "string";
  Task::attributes["entry"] = "date";
  Task::attributes["tags"] = "string";
  Task::attributes["uuid"] = "string";

  good = true;
  try {Task t4 ("{}");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('{}')");

  good = true;
  try {Task t5 ("{\"uuid\":\"00000000-0000-0000-000000000001\",\"description\":\"foo\",\"entry\":\"1234567890\"}");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('{<minimal>}')");

  // Verify tag handling is correct between F4 and JSON.
  Task t6;
  t6.set ("entry", "20130602T224000Z");
  t6.set ("description", "DESC");
  t6.addTag ("tag1");
  test.is (t6.composeF4 (), "[description:\"DESC\" entry:\"20130602T224000Z\" tags:\"tag1\"]", "F4 good");
  test.is (t6.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\"]}", "JSON good");

  t6.addTag ("tag2");
  test.is (t6.composeF4 (), "[description:\"DESC\" entry:\"20130602T224000Z\" tags:\"tag1,tag2\"]", "F4 good");
  test.is (t6.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}", "JSON good");

  good = true;
  Task t7;
  try {t7 = Task ("{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}");}
  catch (const std::string& e){test.diag (e); good = false;}
  test.ok (good, "Task::Task ('{two tags}')");
  test.is (t7.composeF4 (), "[description:\"DESC\" entry:\"1370212800\" tags:\"tag1,tag2\"]", "F4 good");
  test.is (t7.composeJSON (), "{\"description\":\"DESC\",\"entry\":\"20130602T224000Z\",\"tags\":[\"tag1\",\"tag2\"]}", "JSON good");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

