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
#include <Nibbler.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (81);

  Nibbler n;
  std::string s;
  int i;

  // Make sure the nibbler behaves itself with trivial input.
  t.diag ("Test all nibbler calls given empty input");
  n = Nibbler ("");
  t.notok (n.getUntil (' ', s),        "trivial: getUntil");
  t.notok (n.getUntil ("hi", s),       "trivial: getUntil");
  t.notok (n.getUntilOneOf ("ab", s),  "trivial: getUntilOneOf");
  t.notok (n.skipN (123),              "trivial: skipN");
  t.notok (n.skip ('x'),               "trivial: skip");
  t.notok (n.skipAll ('x'),            "trivial: skipAll");
  t.notok (n.skipAllOneOf ("abc"),     "trivial: skipAllOneOf");
  t.notok (n.getQuoted ('"', s),       "trivial: getQuoted");
  t.notok (n.getInt (i),               "trivial: getInt");
  t.notok (n.getUnsignedInt (i),       "trivial: getUnsignedInt");
  t.notok (n.getUntilEOL (s),          "trivial: getUntilEOL");
//
  t.notok (n.getUntilEOS (s),          "trivial: getUntilEOS");
  t.ok    (n.depleted (),              "trivial: depleted");

  // bool getUntil (char, std::string&);
  t.diag ("Nibbler::getUntil");
  n = Nibbler ("one two");
  t.ok    (n.getUntil (' ', s),        " 'one two' :       getUntil (' ')    -> true");
  t.is    (s, "one",                   " 'one two' :       getUntil (' ')    -> 'one'");
  t.ok    (n.getUntil (' ', s),        "    ' two' :       getUntil (' ')    -> true");
  t.is    (s, "",                      "    ' two' :       getUntil (' ')    -> ''");
  t.ok    (n.skip (' '),               "    ' two' :           skip (' ')    -> true");
  t.ok    (n.getUntil (' ', s),        "     'two' :       getUntil (' ')    -> 'two'");
  t.notok (n.getUntil (' ', s),        "        '' :       getUntil (' ')    -> false");
  t.ok    (n.depleted (),              "        '' :       depleted ()       -> true");

  // bool getUntilOneOf (const std::string&, std::string&);
  t.diag ("Nibbler::getUntilOneOf");
  n = Nibbler ("ab.cd");
  t.ok    (n.getUntilOneOf (".:", s),  "   'ab.cd' :  getUntilOneOf ('.:')   -> true");
  t.is    (s, "ab",                    "   'ab.cd' :  getUntilOneOf ('.:')   -> 'ab'");
  t.ok    (n.skipN (),                 "     '.cd' :          skipN ()       -> true");
  t.ok    (n.getUntilOneOf (".:", s),  "      'cd' :  getUntilOneOf ('.:')   -> true");
  t.notok (n.getUntilOneOf (".:", s),  "        '' :  getUntilOneOf ('.:')   -> false");
  t.ok    (n.depleted (),              "        '' :       depleted ()       -> true");

  // bool getUntil (const std::string&, std::string&);
  t.diag ("Nibbler::getUntil");
  n = Nibbler ("ab\r\ncd");
  t.ok (n.getUntil ("\r\n", s),     "'ab\\r\\ncd' :       getUntil ('\\r\\n') -> true");
  t.ok (n.skipN (2),                "  '\\r\\ncd' :          skipN (2)      -> true");
  t.ok (n.getUntil ("\r\n", s),     "      'cd' :       getUntil ('\\r\\n') -> true");
  t.ok (n.depleted (),              "        '' :       depleted ()       -> true");

  // bool skipN (const int quantity = 1);
  t.diag ("Nibbler::skipN");
  n = Nibbler ("abcde");
  t.ok    (n.skipN (),              "   'abcde' :          skipN ()       -> true");
  t.ok    (n.skipN (2),             "    'bcde' :          skipN (2       -> true");
  t.notok (n.skipN (3),             "      'de' :          skipN (3       -> false");
  t.notok (n.depleted (),           "      'de' :       depleted ()       -> true");

  // bool skip (char);
  t.diag ("Nibbler::skip");
  n = Nibbler ("  a");
  t.ok    (n.skip (' '),            "     '  a' :           skip (' ')    -> true");
  t.ok    (n.skip (' '),            "      ' a' :           skip (' ')    -> true");
  t.notok (n.skip (' '),            "       'a' :           skip (' ')    -> false");
  t.notok (n.depleted (),           "       'a' :       depleted ()       -> false");
  t.ok    (n.skip ('a'),            "       'a' :           skip ('a')    -> true");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool skipAll (char);
  t.diag ("Nibbler::skipAll");
  n = Nibbler ("aaaabb");
  t.ok    (n.skipAll ('a'),         "  'aaaabb' :        skipAll ('a')    -> true");
  t.notok (n.skipAll ('a'),         "      'bb' :        skipAll ('a')    -> false");
  t.ok    (n.skipAll ('b'),         "      'bb' :        skipAll ('b')    -> true");
  t.notok (n.skipAll ('b'),         "        '' :        skipAll ('b')    -> false");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool skipAllOneOf (const std::string&);
  t.diag ("Nibbler::skipAllOneOf");
  n = Nibbler ("abababcc");
  t.ok    (n.skipAllOneOf ("ab"),   "'abababcc' :   skipAllOneOf ('ab')   -> true");
  t.notok (n.skipAllOneOf ("ab"),   "      'cc' :   skipAllOneOf ('ab')   -> false");
  t.ok    (n.skipAllOneOf ("c"),    "      'cc' :   skipAllOneOf ('ab')   -> false");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool getQuoted (char, std::string&);
  t.diag ("Nibbler::getQuoted");
  n = Nibbler ("''");
  t.ok    (n.getQuoted ('\'', s),   "      '''' :      getQuoted (''')    -> true");
  t.is    (s, "",                   "      '''' :      getQuoted (''')    -> ''");

  n = Nibbler ("'\"'");
  t.ok    (n.getQuoted ('\'', s),   "      ''\"'' :     getQuoted (''')    -> true");
  t.is    (s, "\"",                  "     ''\"'' :      getQuoted (''')    -> '\"'");

  n = Nibbler ("'x'");
  t.ok    (n.getQuoted ('\'', s),   "      ''x'' :     getQuoted (''')    -> true");
  t.is    (s, "x",                  "      ''x'' :     getQuoted (''')    -> ''");

  n = Nibbler ("'x");
  t.notok (n.getQuoted ('\'', s),   "      ''x' :      getQuoted (''')    -> false");

  n = Nibbler ("x");
  t.notok (n.getQuoted ('\'', s),   "       'x' :      getQuoted (''')    -> false");

  // bool getInt (int&);
  t.diag ("Nibbler::getInt");
  n = Nibbler ("123 -4");
  t.ok    (n.getInt (i),            "  '123 -4' :         getInt ()       -> true");
  t.is    (i, 123,                  "  '123 -4' :         getInt ()       -> '123'");
  t.ok    (n.skip (' '),            "     ' -4' :           skip (' ')    -> true");
  t.ok    (n.getInt (i),            "      '-4' :         getInt ()       -> true");
  t.is    (i, -4,                   "      '-4' :         getInt ()       -> '-4'");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool getUnsignedInt (int&i);
  t.diag ("Nibbler::getUnsignedInt");
  n = Nibbler ("123 4");
  t.ok    (n.getUnsignedInt (i),    "   '123 4' : getUnsignedInt ()       -> true");
  t.is    (i, 123,                  "   '123 4' : getUnsignedInt ()       -> '123'");
  t.ok    (n.skip (' '),            "      ' 4' :           skip (' ')    -> true");
  t.ok    (n.getUnsignedInt (i),    "       '4' : getUnsignedInt ()       -> true");
  t.is    (i, 4,                    "       '4' : getUnsignedInt ()       -> '4'");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool getUntilEOL (std::string&);
  t.diag ("Nibbler::getUntilEOL");
  n = Nibbler ("one\ntwo");
  t.ok    (n.getUntilEOL (s),       "'one\\ntwo' :    getUntilEOL ()       -> true");
  t.is    (s, "one",                "'one\\ntwo' :    getUntilEOL ()       -> 'one'");
  t.ok    (n.skip ('\n'),           "   '\\ntwo' :           skip ('\\n')   -> true");
  t.ok    (n.getUntilEOL (s),       "     'two' :    getUntilEOL ()       -> true");
  t.is    (s, "two",                "     'two' :    getUntilEOL ()       -> 'two'");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool getUntilEOS (std::string&);
  t.diag ("Nibbler::getUntilEOS");
  n = Nibbler ("one two");
  t.ok    (n.getUntilEOS (s),       " 'one two' :    getUntilEOS ()       -> 'one two'");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  // bool depleted ();
  t.diag ("Nibbler::depleted");
  n = Nibbler (" ");
  t.notok (n.depleted (),           "       ' ' :       depleted ()       -> false");
  t.ok    (n.skipN (),              "        '' :           skip ()       -> true");
  t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
