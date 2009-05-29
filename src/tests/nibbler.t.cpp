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
#include <Nibbler.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (52);

  Nibbler n ("this is 'a test' of 123the,nibbler");
  std::string s;
  int i;

  t.ok (n.getUntilChar (' ', s),    "nibble word");
  t.is (s, "this",                  "found 'this'");
  t.ok (n.skip (' '),               "skip ws");
  t.ok (n.getUntilChar (' ', s),    "nibble word");
  t.is (s, "is",                    "found 'is'");
  t.ok (n.skip (' '),               "skip ws");
  t.ok (n.getQuoted ('\'', s),      "nibble quoted");
  t.is (s, "a test",                "found 'a test'");
  t.ok (n.skip (' '),               "skip ws");
  t.ok (n.getUntilChar (' ', s),    "nibble word");
  t.is (s, "of",                    "found 'of'");
  t.ok (n.skip (' '),               "skip ws");
  t.ok (n.getInt (i),               "nibble integer");
  t.is (i, 123,                     "found '123'");
  t.ok (n.getUntilChar (',', s),    "nibble word");
  t.ok (n.skip (','),               "skip ,");
  t.is (s, "the",                   "found 'the'");
  t.ok (n.getUntilEOS (s),          "nibble remainder");
  t.is (s, "nibbler",               "found 'nibbler'");

  // Test EOS handling.
  n = Nibbler ("xx");
  t.ok (n.skip ('x'),               "skip x");
  t.ok (n.skip ('x'),               "skip x");
  t.notok (n.skip ('x'),            "skip x");

  n = Nibbler ("aaaaaX");
  t.ok (n.skip (5),                 "aaaaaX -> skip 5 pass");
  t.notok (n.skip(2),               "X -> skip 2 fail");
  t.ok (n.skip (),                  "X -> skip pass");

  n = Nibbler ("aaaaaa");
  t.ok (n.skipAll ('a'),            "aaaaaa -> skipAll 'a' pass");

  n = Nibbler ("aabbabab");
  t.ok (n.skipAllChars ("ab"),      "aabbabab -> skipAllChars 'ab' pass");

  n = Nibbler ("abcX");
  t.ok (n.skipAllChars ("abc"),        "abcX -> skipChars abc pass");
  t.notok (n.skipAllChars ("abc"),     "X -> skipChars abc fail");

  n = Nibbler ("-123+456 789");
  t.ok (n.getInt (i),               "-123+456 789 -> getInt pass");
  t.is (i, -123,                    "-123+456 789 -> -123 pass");
  t.notok (n.getUnsignedInt (i),    "+456 789 -> getUnsignedInt fail");
  t.ok (n.getInt (i),               "+456 789 -> getInt pass");
  t.is (i, 456,                     "+456 789 -> getInt pass");
  t.ok (n.skip (' '),               "\s789 -> skip ' ' pass");
  t.ok (n.getUnsignedInt (i),       "789 -> getUnsignedInt pass");
  t.is (i, 789,                     "789 -> getInt pass");

  n = Nibbler ("123");
  t.ok (n.getInt (i),               "123 -> getInt pass");
  t.is (i, 123,                     "123 -> getInt 123 pass");

  n = Nibbler ("abc\nd");
  t.ok (n.getUntilEOL (s),          "abc\\nd -> getUntilEOL pass");
  t.is (s, "abc",                   "abc\\nd -> getUntilEOL abc pass");

  n = Nibbler ("abcba'foo");
  t.ok (n.getQuoted ('a', s),       "abcba'foo -> getQuoted 'a' pass");
  t.is (s, "bcb",                   "abcba'foo -> getQuoted 'a' bcb pass");
  t.notok (n.getQuoted ('\'', s),   "'foo -> getQuoted '\\'' fail");

  n = Nibbler ("abcde");
  t.ok (n.getUntilChars ("ed", s),  "abcde -> getUntilChars 'ed' pass");
  t.is (s, "abc",                   "abcde -> getUntilChars 'ed abc pass");

  n = Nibbler ("abcde");
  t.ok (n.getUntilString ("de", s), "abcde -> getUntilString 'de' pass");
  t.is (s, "abc",                   "abcde -> getUntilString 'de abc pass");

  n = Nibbler ("aa");
  t.notok (n.depleted (),           "'aa' -> not depleted pass");
  t.ok (n.skip ('a'),               "aa -> skip 'a' pass");
  t.ok (n.skip ('a'),               "aa -> skip 'a' pass");
  t.ok (n.depleted (),              "'' -> depleted pass");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
