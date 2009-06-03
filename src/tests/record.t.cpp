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
#include <Context.h>
#include <Att.h>
#include <Record.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (11);

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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
