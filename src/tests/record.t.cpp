////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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
#include <Context.h>
#include <Att.h>
#include <Record.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (19);

  // (blank)
  bool good = true;
  Record record;

  try {record = Record ("");}
  catch (std::string& e){t.diag (e); good = false;}
  t.notok (good, "Record::Record ('')");

  // []
  good = true;
  try {record = Record ("[]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.notok (good, "Record::Record ('[]')");

  // [name:value]
  good = true;
  try {record = Record ("[name:value]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Record::Record ('[name:value]')");
  t.is (record.get ("name"), "value", "name=value");

  // [name:"value"]
  good = true;
  try {record = Record ("[name:\"value\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Record::Record ('[name:\"value\"]')");
  t.is (record.get ("name"), "value", "name=value");

  // [name:"one two"]
  good = true;
  try {record = Record ("[name:\"one two\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Record::Record ('[name:\"one two\"]')");
  t.is (record.get ("name"), "one two", "name=one two");

  // [one:two three:four]
  good = true;
  try {record = Record ("[one:\"two\" three:\"four\"]");}
  catch (std::string& e){t.diag (e); good = false;}
  t.ok (good, "Record::Record ('[one:\"two\" three:\"four\"]')");
  t.is (record.get ("one"), "two", "one=two");
  t.is (record.get ("three"), "four", "three=four");

  // Record::set
  record.clear ();
  record.set ("name", "value");
  t.is (record.composeF4 (), "[name:\"value\"]\n", "Record::set");

  // Record::has
  t.ok    (record.has ("name"), "Record::has");
  t.notok (record.has ("woof"), "Record::has not");

  // Record::get_int
  record.set ("one", 1);
  t.is (record.composeF4 (), "[name:\"value\" one:\"1\"]\n", "Record::set");
  t.is (record.get_int ("one"), 1, "Record::get_int");

  // Record::remove
  record.remove ("one");
  t.is (record.composeF4 (), "[name:\"value\"]\n", "Record::remove");

  // Record::all
  std::vector <Att> all = record.all ();
  t.is (all.size (), (size_t)1, "Record::all size");
  t.is (all[0].name (), "name", "Record::all[0].name ()");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
