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
  t.ok (n.getUntil (' ', s),       "'one two' :      getUntil (' ')  -> true");
  t.is (s, "one",                  "'one two' :      getUntil (' ')  -> 'one'");
  t.ok (n.getUntil (' ', s),       "   ' two' :      getUntil (' ')  -> true");
  t.is (s, "",                     "   ' two' :      getUntil (' ')  -> ''");
  t.ok (n.skip (' '),              "   ' two' :          skip (' ')  -> true");
  t.ok (n.getUntil (' ', s),       "    'two' :      getUntil (' ')  -> 'two'");
  t.ok (n.getUntil (' ', s),       "       '' :      getUntil (' ')  -> false");
  t.ok (n.depleted (),             "       '' :      depleted ()     -> true");

  // bool getUntilOneOf (const std::string&, std::string&);
  t.diag ("Nibbler::getUntilOneOf");
  n = Nibbler ("ab.cd");
  t.ok (n.getUntilOneOf (".:", s), "  'ab.cd' : getUntilOneOf ('.:') -> true");
  t.is (s, "ab",                   "  'ab.cd' : getUntilOneOf ('.:') -> 'ab'");
  t.ok (n.skipN (),                "    '.cd' :         skipN ()     -> true");
  t.ok (n.getUntilOneOf (".:", s), "     'cd' : getUntilOneOf ('.:') -> true");
//
  t.ok (n.getUntilOneOf (".:", s), "       '' : getUntilOneOf ('.:') -> false");
  t.ok (n.depleted (),             "       '' :      depleted ()     -> true");

  // bool getUntil (const std::string&, std::string&);
  t.diag ("Nibbler::getUntil");
  n = Nibbler ("ab\r\ncd");

  // bool skipN (const int quantity = 1);
  t.diag ("Nibbler::skipN");
  n = Nibbler ("abcde");
  t.ok    (n.skipN (),             "  'abcde' :         skipN ()     -> true");
  t.ok    (n.skipN (2),            "   'bcde' :         skipN (2     -> true");
  t.notok (n.skipN (3),            "     'de' :         skipN (3     -> false");
  t.notok (n.depleted (),          "     'de' :      depleted ()     -> true");

  // bool skip (char);
  t.diag ("Nibbler::skip");
  n = Nibbler ("  a");
  t.ok    (n.skip (' '),           "    '  a' :          skip (' ')  -> true");
  t.ok    (n.skip (' '),           "     ' a' :          skip (' ')  -> true");
  t.notok (n.skip (' '),           "      'a' :          skip (' ')  -> false");
  t.notok (n.depleted (),          "      'a' :      depleted ()     -> false");
//
  t.notok (n.skip ('a'),           "       '' :          skip ('a')  -> true");
  t.ok    (n.depleted (),          "       '' :      depleted ()     -> true");

  // bool skipAll (char);
  t.diag ("Nibbler::skipAll");
  n = Nibbler ("aaaabb");
  t.ok    (n.skipAll ('a'),        " 'aaaabb' :       skipAll ('a')  -> true");
  t.notok (n.skipAll ('a'),        "     'bb' :       skipAll ('a')  -> false");
  t.ok    (n.skipAll ('b'),        "     'bb' :       skipAll ('b')  -> true");
  t.notok (n.skipAll ('b'),        "       '' :       skipAll ('b')  -> false");
  t.ok    (n.depleted (),          "       '' :      depleted ()     -> true");

  // bool skipAllOneOf (const std::string&);
  t.diag ("Nibbler::skipAllOneOf");

  // bool getQuoted (char, std::string&);
  t.diag ("Nibbler::getQuoted");

  // bool getInt (int&);
  t.diag ("Nibbler::getInt");

  // bool getUnsignedInt (int&i);
  t.diag ("Nibbler::getUnsignedInt");

  // bool getUntilEOL (std::string&);
  t.diag ("Nibbler::getUntilEOL");

  // bool getUntilEOS (std::string&);
  t.diag ("Nibbler::getUntilEOS");

  // bool depleted ();
  t.diag ("Nibbler::depleted");
  n = Nibbler (" ");
  t.notok (n.depleted (),          "      ' ' :      depleted ()     -> false");
  t.ok    (n.skipN (),             "       '' :          skip ()     -> true");
  t.ok    (n.depleted (),          "       '' :      depleted ()     -> true");

  return 0;

/*
  n = Nibbler ("this is 'a test' of 123the,nibbler");
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
  t.ok (n.skip (' '),               "\\s789 -> skip ' ' pass");
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
*/
}

////////////////////////////////////////////////////////////////////////////////
