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
#include <Att.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (33);

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
  try {a6.addMod ("is");} catch (...) {t.fail ("Att::addMod (is)"); good = false;}
  if (good) t.pass ("Att::addMod (is)");

  good = true;
  try {a6.addMod (Mod ("fartwizzle"));} catch (...) {t.pass ("Att::addMod (fartwizzle) failed"); good = false;}
  if (good) t.fail ("Att::addMod (fartwizzle)");

  // Att::parse
  Att a7;
  a7.parse ("");
  t.is (a7.composeF4 (), "", "Att::composeF4 ()");

  a7.parse ("name:value");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:value)");

  a7.parse ("name:\"value\"");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:\"value\")");

  a7.parse ("name:\"one two\"");
  t.is (a7.composeF4 (), "name:\"one two\"", "Att::composeF4 (name:\"one two\")");

  a7.parse ("name:\"&quot;\"");
  t.is (a7.composeF4 (), "name:\"&quot;\"", "Att::composeF4 (name:\"&quot;\")");

  a7.parse ("name:\"&tab;&quot;&comma;&open;&close;&colon;\"");
  t.is (a7.composeF4 (), "name:\"&tab;&quot;&comma;&open;&close;&colon;\"",
                         "Att::composeF4 (name:\"&tab;&quot;&comma;&open;&close;&colon;\")");

  a7.parse ("total gibberish");
  t.is (a7.composeF4 (), "", "Att::composeF4 (total gibberish)");

  a7.parse ("malformed");
  t.is (a7.composeF4 (), "", "Att::composeF4 (malformed)");

  a7.parse (":malformed");
  t.is (a7.composeF4 (), "", "Att::composeF4 (:malformed)");

  a7.parse (":\"\"");
  t.is (a7.composeF4 (), "", "Att::composeF4 (:\"\")");

  a7.parse (":\"");
  t.is (a7.composeF4 (), "", "Att::composeF4 (:\")");

  a7.parse ("name:");
  t.is (a7.composeF4 (), "", "Att::composeF4 (name:)");

  a7.parse ("name:\"value");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:\"value)");

  a7.parse ("name:value\"");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:value\")");

  a7.parse ("name:val\"ue");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:val\"ue)");

  a7.parse ("name:\"\"va\"\"\"\"\"lu\"\"\"e\"\"");
  t.is (a7.composeF4 (), "name:\"value\"", "Att::composeF4 (name:\"\"va\"\"\"\"\"lu\"\"\"e\"\")");

  a7.parse ("name\"");
  t.is (a7.composeF4 (), "", "Att::composeF4 (name\")");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
