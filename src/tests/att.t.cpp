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
  UnitTest t (67);

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

  // Att::mod
  bool good = true;
  try {a6.mod ("is");} catch (...) {good = false;}
  t.ok (good, "Att::mod (is)");

  good = true;
  try {a6.mod ("before");} catch (...) {good = false;}
  t.ok (good, "Att::mod (before)");

  good = true;
  try {a6.mod ("after");} catch (...) {good = false;}
  t.ok (good, "Att::mod (after)");

  good = true;
  try {a6.mod ("none");} catch (...) {good = false;}
  t.ok (good, "Att::mod (none)");

  good = true;
  try {a6.mod ("any");} catch (...) {good = false;}
  t.ok (good, "Att::mod (any)");

  good = true;
  try {a6.mod ("over");} catch (...) {good = false;}
  t.ok (good, "Att::mod (over)");

  good = true;
  try {a6.mod ("under");} catch (...) {good = false;}
  t.ok (good, "Att::mod (under)");

  good = true;
  try {a6.mod ("above");} catch (...) {good = false;}
  t.ok (good, "Att::mod (above)");

  good = true;
  try {a6.mod ("below");} catch (...) {good = false;}
  t.ok (good, "Att::mod (below)");

  good = true;
  try {a6.mod ("isnt");} catch (...) {good = false;}
  t.ok (good, "Att::mod (isnt)");

  good = true;
  try {a6.mod ("has");} catch (...) {good = false;}
  t.ok (good, "Att::mod (has)");

  good = true;
  try {a6.mod ("contains");} catch (...) {good = false;}
  t.ok (good, "Att::mod (contains)");

  good = true;
  try {a6.mod ("hasnt");} catch (...) {good = false;}
  t.ok (good, "Att::mod (hasnt)");

  good = true;
  try {a6.mod ("startswith");} catch (...) {good = false;}
  t.ok (good, "Att::mod (startswith)");

  good = true;
  try {a6.mod ("endswith");} catch (...) {good = false;}
  t.ok (good, "Att::mod (endswith)");

  good = true;
  try {a6.mod ("fartwizzle");} catch (...) {good = false;}
  t.notok (good, "Att::mod (fartwizzle)");

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

  n = Nibbler ("name.bogus:\"value\"");
  good = true;
  try {a7.parse (n);} catch (...) {good = false;}
  t.notok (good, "Att::parse (name.bogus:\"value\")");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
