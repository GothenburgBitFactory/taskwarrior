////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <Context.h>
#include <Task.h>
#include <Taskmod.h>
#include <sys/time.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (16);

  bool good = true;

  // base timestamp
  unsigned long timestamp = (unsigned long)time(NULL);
  timestamp -= timestamp % 100;

  // create some tasks
  Task tasks[3];

  // base task
  tasks[0] = Task("[description:\"Desc1\" uuid:\"df95dac3-5f2b-af88-5416-03a3163d00fd\"]");
  // first modification
  tasks[1] = tasks[0];
  tasks[1].addTag("tag1");
  // second modification
  tasks[2] = tasks[1];
  tasks[2].setStatus(Task::completed);

  // create taskmods
  Taskmod mods[4];
  mods[0] = Taskmod();
  mods[0].setTimestamp(timestamp);
  mods[0].setBefore(tasks[0]);
  mods[0].setAfter(tasks[1]);

  mods[1] = Taskmod();
  mods[1].setTimestamp(timestamp + 2);
  mods[1].setBefore(tasks[1]);
  mods[1].setAfter(tasks[2]);

  // getUuid() not
  Taskmod empty = Taskmod();
  good = true;
  try {
	  empty.getUuid();
  } catch (const std::string& e) { t.diag(e); good = false; }
  t.notok (good, "Taskmod::getUuid() not");

  // issetAfter() not
  Taskmod newMod = Taskmod();
  t.notok(newMod.issetAfter(), "Taskmod::issetAfter() not");

  // getUuid()
  newMod.setAfter(tasks[1]);
  try {
	 std::string uuid = newMod.getUuid();
	 t.is(uuid, "df95dac3-5f2b-af88-5416-03a3163d00fd", "Taskmod::getUuid()");
  } catch (const std::string& e) {
    t.diag(e);
	 t.fail("Taskmod::getUuid()");
  }

  // isValid() not
  t.notok(newMod.isValid(), "Taskmod::isValid() not") ;

  // issetBefore() not
  t.notok(newMod.issetBefore(), "Taskmod::issetBefore() not");

  // isValid()
  newMod.setTimestamp(timestamp+1);
  t.ok(newMod.isValid(), "Taskmod::isValid()");

  // isNew()
  t.ok(newMod.isNew(), "Taskmod::isNew()");

  // issetBefore()
  newMod.setBefore(tasks[0]);
  t.ok(newMod.issetBefore(), "Taskmod::issetBefore()");

  // isNew() not
  t.notok(newMod.isNew(), "Taskmod::isNew() not");

  // <
  t.ok(mods[0] < newMod, "Taskmod::operator<");
  t.notok(newMod < mods[0], "Taskmod::operator< not");

  // >
  t.ok(mods[1] > mods[0], "Taskmod::operator>");
  t.notok(mods[0] > mods[1], "Taskmod::operator> not");

  // !=
  t.ok(mods[0] != mods[1], "Taskmod::operator!=" );

  // copy constructor
  Taskmod clone1 = Taskmod(mods[0]);
  t.ok(mods[0] == clone1, "Taskmod::Taskmod(const Taskmod&)");

  // =
  Taskmod clone2 = mods[0];
  t.ok(mods[0] == clone2, "Taskmod::operator=");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
