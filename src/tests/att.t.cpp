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
#include <Context.h>
#include <Att.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (74);

  Att a;
  t.notok (a.valid ("name"),            "Att::valid name         -> fail");
  t.notok (a.valid (":"),               "Att::valid :            -> fail");
  t.notok (a.valid (":value"),          "Att::valid :value       -> fail");

  t.ok (a.valid ("name:value"),         "Att::valid name:value");
  t.ok (a.valid ("name:value "),        "Att::valid name:value\\s");
  t.ok (a.valid ("name:'value'"),       "Att::valid name:'value'");
  t.ok (a.valid ("name:'one two'"),     "Att::valid name:'one two'");
  t.ok (a.valid ("name:\"value\""),     "Att::valid name:\"value\"");
  t.ok (a.valid ("name:\"one two\""),   "Att::valid name:\"one two\"");
  t.ok (a.valid ("name:"),              "Att::valid name:");
  t.ok (a.valid ("name:&quot;"),        "Att::valid &quot;");
  t.ok (a.valid ("name.one:value"),     "Att::valid name.one.value");
  t.ok (a.valid ("name.one.two:value"), "Att::valid name.one.two:value");
  t.ok (a.valid ("name.:value"),        "Att::valid name.:value");
  t.ok (a.valid ("name..:value"),       "Att::valid name..:value");

  Att a1 ("name", "value");
  t.is (a1.name (), "name", "Att::Att (name, value), Att.name");
  t.is (a1.value (), "value", "Att::Att (name, value), Att.value");

  Att a2;
  a2.name ("name");
  a2.value ("value");
  t.is (a2.name (), "name", "Att::Att (), Att.name");
  t.is (a2.value (), "value", "Att::Att (), Att.value");

  Att a3 (a2);
  t.is (a3.name (), "name", "Att::Att (Att), Att.name");
  t.is (a3.value (), "value", "Att::Att (Att), Att.value");

  Att a4;
  a4 = a2;
  t.is (a4.name (), "name", "Att::Att (), Att.operator=, Att.name");
  t.is (a4.value (), "value", "Att::Att (), Att.operator=, Att.value");

  Att a5 ("name", "value");
  t.is (a5.composeF4 (), "name:\"value\"", "Att::composeF4 simple");
  a5.value ("\"");
  t.is (a5.composeF4 (), "name:\"&quot;\"", "Att::composeF4 encoded \"");
  a5.value ("\t\",[]:");
  t.is (a5.composeF4 (), "name:\"&tab;&quot;&comma;&open;&close;&colon;\"", "Att::composeF4 fully encoded \\t\",[]:");

  Att a6 ("name", 6);
  t.is (a6.value_int (), 6, "Att::value_int get");
  a6.value_int (7);
  t.is (a6.value_int (), 7, "Att::value_int set/get");
  t.is (a6.value (), "7", "Att::value 7");

  // Att::addMod
  bool good = true;
  try {a6.addMod ("is");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (is)");

  good = true;
  try {a6.addMod ("before");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (before)");

  good = true;
  try {a6.addMod ("after");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (after)");

  good = true;
  try {a6.addMod ("not");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (not)");

  good = true;
  try {a6.addMod ("none");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (none)");

  good = true;
  try {a6.addMod ("any");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (any)");

  good = true;
  try {a6.addMod ("over");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (over)");

  good = true;
  try {a6.addMod ("under");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (under)");

  good = true;
  try {a6.addMod ("synth");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (synth)");

  good = true;
  try {a6.addMod ("first");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (first)");

  good = true;
  try {a6.addMod ("last");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (last)");

  good = true;
  try {a6.addMod ("this");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (this)");

  good = true;
  try {a6.addMod ("next");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (next)");

  good = true;
  try {a6.addMod ("isnt");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (isnt)");

  good = true;
  try {a6.addMod ("has");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (has)");

  good = true;
  try {a6.addMod ("hasnt");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (hasnt)");

  good = true;
  try {a6.addMod ("startswith");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (startswith)");

  good = true;
  try {a6.addMod ("endswith");} catch (...) {good = false;}
  t.ok (good, "Att::addMod (endswith)");

  good = true;
  try {a6.addMod ("fartwizzle");} catch (...) {good = false;}
  t.notok (good, "Att::addMod (fartwizzle)");

  // Att::mods
  std::vector <std::string> mods;
  a6.mods (mods);
  t.is (mods.size (), (size_t)18, "Att::mods () size == 18");
  t.is (mods[0], "is",            "Att::mods [0] == 'is'");

  // Att::parse
  Nibbler n ("");
  Att a7;
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse () -> throw");

  n = Nibbler ("name:value");
  a7.parse (n);
  t.is (a7.composeF4 (), "name:\"value\"",
         "Att::parse (name:value)");

  n = Nibbler ("name:\"value\"");
  a7.parse (n);
  t.is (a7.composeF4 (), "name:\"value\"",
         "Att::parse (name:\"value\")");

  n = Nibbler ("name:\"one two\"");
  a7.parse (n);
  t.is (a7.composeF4 (), "name:\"one two\"",
         "Att::parse (name:\"one two\")");

  n = Nibbler ("name:\"&quot;\"");
  a7.parse (n);
  t.is (a7.composeF4 (), "name:\"&quot;\"",
         "Att::parse (name:\"&quot;\")");

  n = Nibbler ("name:\"&tab;&quot;&comma;&open;&close;&colon;\"");
  a7.parse (n);
  t.is (a7.composeF4 (), "name:\"&tab;&quot;&comma;&open;&close;&colon;\"",
             "Att::parse (name:\"&tab;&quot;&comma;&open;&close;&colon;\")");

  n = Nibbler ("total gibberish");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (total gibberish)");

  n = Nibbler ("malformed");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (malformed)");

  n = Nibbler (":malformed");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (:malformed)");

  n = Nibbler (":\"\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (:\"\")");

  n = Nibbler (":\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (:\")");

  n = Nibbler ("name:");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (name:)");

  n = Nibbler ("name:\"value");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.ok (good, "Att::parse (name:\"value)");
  t.is (a7.composeF4 (), "name:\"&quot;value\"", "Att::composeF4 -> name:\"&quot;value\"");

  n = Nibbler ("name:value\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.ok (good, "Att::parse (name:value\")");
  t.is (a7.composeF4 (), "name:\"value&quot;\"", "Att::composeF4 -> name:\"value&quot;\"");

  n = Nibbler ("name:val\"ue");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.ok (good, "Att::parse (name:val\"ue)");
  t.is (a7.composeF4 (), "name:\"val&quot;ue\"", "Att::composeF4 -> name:\"val&quot;ue\"");

  n = Nibbler ("name\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (name\")");

  // Mods
  n = Nibbler ("name.any:\"value\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.ok (good, "Att::parse (name.any:\"value\")");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 -> name:\"value\"");

  n = Nibbler ("name.any.none:\"value\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.ok (good, "Att::parse (name.any.none:\"value\")");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 -> name:\"value\"");

  n = Nibbler ("name.bogus:\"value\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (name.bogus:\"value\")");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
