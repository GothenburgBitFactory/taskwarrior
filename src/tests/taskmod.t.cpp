////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <Context.h>
//#include <Att.h>
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
  } catch (std::string& e) { t.diag(e); good = false; }
  t.notok (good, "Taskmod::getUuid() not");

  // issetAfter() not
  Taskmod newMod = Taskmod();
  t.notok(newMod.issetAfter(), "Taskmod::issetAfter() not");

  // getUuid()
  newMod.setAfter(tasks[1]);
  try {
	 std::string uuid = newMod.getUuid();
	 t.is(uuid, "df95dac3-5f2b-af88-5416-03a3163d00fd", "Taskmod::getUuid()");
  } catch (std::string& e) {
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
