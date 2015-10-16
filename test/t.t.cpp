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
#include <stdlib.h>
#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest test (23);

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

  // Task::validate
  Task bad ("[entry:1000000001 start:1000000000]");
  good = true;
  try { bad.validate (); } catch (...) { good = false; }
  test.notok (good, "Task::validate entry <= start");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

