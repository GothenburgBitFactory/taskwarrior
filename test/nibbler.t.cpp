////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
  UnitTest t (162);

  try
  {
    Nibbler n;
    std::string s;
    int i;
    double d;

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
    t.notok (n.getHex (i),               "trivial: getHex");
    t.notok (n.getUnsignedInt (i),       "trivial: getUnsignedInt");
    t.notok (n.getUntilEOL (s),          "trivial: getUntilEOL");
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

    // bool getUntilRx (const std::string&, std::string&);
    t.diag ("Nibbler::getUntilRx");
    n = Nibbler ("one two");
    t.ok    (n.getUntilRx ("th", s),     " 'one two' :     getUntilRx ('th')   -> true");
    t.is    (s, "one two",               " 'one two' :     getUntilRx ('th')   -> 'one two'");

    n = Nibbler ("one two");
    t.ok    (n.getUntilRx ("e", s),      " 'one two' :     getUntilRx ('e')    -> true");
    t.is    (s, "on",                    " 'one two' :     getUntilRx ('e')    -> 'on'");
    t.ok    (n.getUntilRx ("tw", s),     "   'e two' :     getUntilRx ('tw')   -> true");
    t.is    (s, "e ",                    "   'e two' :     getUntilRx ('tw')   -> 'e '");
    t.ok    (n.getUntilRx ("$", s),      "     'two' :     getUntilRx ('$')    -> true");
    t.is    (s, "two",                   "     'two' :     getUntilRx ('$')    -> 'two'");
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

    // bool getUntilWS (std::string&);
    t.diag ("Nibbler::getUntilWS");
    n = Nibbler ("ab \t\ncd");
    t.ok    (n.getUntilWS (s),        " 'ab \\t\\ncd' :     getUntilWS ()     -> true");
    t.is    (s, "ab",                 " 'ab \\t\\ncd' :     getUntilWS ()     -> 'ab'");
    t.ok    (n.getUntilWS (s),        "   ' \\t\\ncd' :     getUntilWS ()     -> true");
    t.is    (s, "",                   "   ' \\t\\ncd' :     getUntilWS ()     -> ''");
    t.ok    (n.skipWS (),             "        'cd' :         skipWS ()     -> true");
    t.ok    (n.getUntilWS (s),        "          '' :     getUntilWS ()     -> true");
    t.is    (s, "cd",                 "        'cd' :     getUntilWS ()     -> 'cd'");
    t.ok    (n.depleted (),           "          '' :       depleted ()     -> true");

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

    // bool skipWS ();
    t.diag ("Nibbler::skipWS");
    n = Nibbler (" \tfoo");
    t.ok    (n.skipWS (),             "  ' \\tfoo' :         skipWS ()       -> true");
    t.notok (n.skipWS (),             "     'foo' :         skipWS ()       -> false");
    t.ok    (n.getUntilEOS (s),       "     'foo' :    getUntilEOS ()       -> true");
    t.is    (s, "foo",                "     'foo' :    getUntilEOS ()       -> 'foo'");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // bool skipRx (const std::string&);
    t.diag ("Nibbler::skipRx");
    n = Nibbler ("one two");
    t.ok    (n.skipRx ("o."),         " 'one two' :         skipRx ('o.')   -> true");
    t.notok (n.skipRx ("A+"),         "   'e two' :         skipRx ('A+')   -> false");
    t.ok    (n.skipRx ("e+"),         "   'e two' :         skipRx ('e+')   -> true");
    t.ok    (n.skipRx ("...."),       "    ' two' :         skipRx ('....') -> true");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // bool getQuoted (char, std::string&);
    t.diag ("Nibbler::getQuoted");
    n = Nibbler ("''");
    t.ok    (n.getQuoted ('\'', s),   "      '''' :      getQuoted (''')    -> true");
    t.is    (s, "",                   "      '''' :      getQuoted (''')    -> ''");

    n = Nibbler ("'\"'");
    t.ok    (n.getQuoted ('\'', s),   "      ''\"'' :     getQuoted (''')    -> true");
    t.is    (s, "\"",                  "     ''\"'' :      getQuoted (''')    -> '\"'"); // 81

    n = Nibbler ("'x'");
    t.ok    (n.getQuoted ('\'', s),   "      ''x'' :     getQuoted (''')    -> true");
    t.is    (s, "x",                  "      ''x'' :     getQuoted (''')    -> ''");     // 83

    n = Nibbler ("'x");
    t.notok (n.getQuoted ('\'', s),   "      ''x' :      getQuoted (''')    -> false");

    n = Nibbler ("x");
    t.notok (n.getQuoted ('\'', s),   "       'x' :      getQuoted (''')    -> false");

    n = Nibbler ("\"one\\\"two\"");
    t.notok (n.getQuoted ('\'', s),   "\"one\\\"two\" :      getQuoted (''')    -> false");             // 86

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s, false), "\"one\\\"two\" :      getQuoted ('\"', false, false) -> true"); // 87
    t.is (s, "one\"two", "getQuoted ('\"', false) -> one\"two");                                        // 88

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s, true),  "\"one\\\"two\" :      getQuoted ('\"', false, true)  -> true"); // 89
    t.is (s, "\"one\"two\"", "getQuoted ('\"', true) -> \"one\"two\"");                                 // 90

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s, false), "\"one\\\"two\" :      getQuoted ('\"', true,  false) -> true"); // 91
    t.is (s, "one\"two", "getQuoted ('\"', false) -> one\"two");                                        // 92

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s, true),  "\"one\\\"two\" :      getQuoted ('\"', true,  true)  -> true"); // 93
    t.is (s, "\"one\"two\"", "getQuoted ('\"', true) -> \"one\"two\"");                                 // 94


    // bool getInt (int&);
    t.diag ("Nibbler::getInt");
    n = Nibbler ("123 -4");
    t.ok    (n.getInt (i),            "  '123 -4' :         getInt ()       -> true");
    t.is    (i, 123,                  "  '123 -4' :         getInt ()       -> '123'");
    t.ok    (n.skip (' '),            "     ' -4' :           skip (' ')    -> true");
    t.ok    (n.getInt (i),            "      '-4' :         getInt ()       -> true");
    t.is    (i, -4,                   "      '-4' :         getInt ()       -> '-4'");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // bool getHex (int&);
    t.diag ("Nibbler::getHex");
    n = Nibbler ("123 7b");
    t.ok    (n.getHex (i),            "  '123 7b' :         getHex ()       -> true");
    t.is    (i, 291,                  "  '123 7b' :         getHex ()       -> '291'");
    t.ok    (n.skip (' '),            "     ' 7b' :           skip (' ')    -> true");
    t.ok    (n.getHex (i),            "      '7b' :         getHex ()       -> true");
    t.is    (i, 123,                  "      '7b' :         getHex ()       -> '123'");
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

    // bool getNumber (double&);
    t.diag ("Nibbler::getNumber");
    n = Nibbler ("-1.234 2.3e4");
    t.ok    (n.getNumber (d),         "'-1.234 2.3e4' : getNumber ()       -> true");
    t.is    (d, -1.234,               "'-1.234 2.3e4' : getNumber ()       -> '-1.234'");
    t.ok    (n.skip (' '),            "      ' 2.3e4' : skip (' ')         -> true");
    t.ok    (n.getNumber (d),         "       '2.3e4' : getNumber ()       -> true");
    t.is    (d, 2.3e4,                "       '2.3e4' : getNumber ()       -> '2.3e4'");
    t.ok    (n.depleted (),           "            '' : depleted ()        -> true");

    // bool getLiteral (const std::string&);
    t.diag ("Nibbler::getLiteral");
    n = Nibbler ("foobar");
    t.ok    (n.getLiteral ("foo"),    "  'foobar' :     getLiteral ('foo')  -> true");
    t.notok (n.getLiteral ("foo"),    "     'bar' :     getLiteral ('foo')  -> false");
    t.ok    (n.getLiteral ("bar"),    "     'bar' :     getLiteral ('bar')  -> true");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // bool getRx (const std::string&, std::string&);
    t.diag ("Nibbler::getRx");
    n = Nibbler ("one two three");
    t.ok    (n.getRx ("^(o..)", s),   "'one two three' :   getRx ('^(o..)')  -> true");
    t.is    (s, "one",                "'one two three' :   getRx ('^(o..)')  -> 'one'");
    t.ok    (n.skip (' '),            "   ' two three' :         skip (' ')  -> true");
    t.ok    (n.getRx ("t..", s),      "    'two three' :   getRx ('t..')     -> true");
    t.is    (s, "two",                "    'two three' :   getRx ('t..')     -> 'two'");
    t.notok (n.getRx ("th...", s),    "       ' three' :   getRx ('th...')   -> false");
    t.ok    (n.skip (' '),            "       ' three' :         skip (' ')  -> true");
    t.ok    (n.getRx ("th...", s),    "        'three' :   getRx ('th...')   -> true");
    t.is    (s, "three",              "        'three' :   getRx ('th...')   -> 'three'");
    t.ok    (n.depleted (),           "             '' :       depleted ()   -> true");

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

    // char next ();
    t.diag ("Nibbler::next");
    n = Nibbler ("hello");
    t.is    (n.next (), 'h',          "   'hello' :           next ()       -> 'h'");
    t.is    (n.next (), 'h',          "   'hello' :           next ()       -> 'h'");
    t.ok    (n.skipN (4),             "   'hello' :          skipN (4)      -> true");
    t.is    (n.next (), 'o',          "       'o' :           next ()       -> 'o'");
    t.ok    (n.skipN (1),             "       'o' :          skipN ()       -> true");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // std::string next (const int quantity);
    t.diag ("Nibbler::next");
    n = Nibbler ("hello");
    t.is    (n.next (1), "h",         "   'hello' :          next (1)       -> 'h'");
    t.is    (n.next (1), "h",         "   'hello' :          next (1)       -> 'h'");
    t.is    (n.next (2), "he",        "   'hello' :          next (2)       -> 'he'");
    t.is    (n.next (3), "hel",       "   'hello' :          next (3)       -> 'hel'");
    t.is    (n.next (4), "hell",      "   'hello' :          next (4)       -> 'hell'");
    t.is    (n.next (5), "hello",     "   'hello' :          next (5)       -> 'hello'");
    t.is    (n.next (6), "",          "   'hello' :          next (6)       -> ''");

    // bool depleted ();
    t.diag ("Nibbler::depleted");
    n = Nibbler (" ");
    t.notok (n.depleted (),           "       ' ' :       depleted ()       -> false");
    t.ok    (n.skipN (),              "        '' :           skip ()       -> true");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // void save ();
    // void restore ();
    n = Nibbler ("abcde");
    t.ok (n.skipN (),                 "   'abcde' :           skip ()       -> true");
    n.save ();
    t.ok (n.skipN (),                 "    'bcde' :           skip ()       -> true");
    t.ok (n.skipN (),                 "     'cde' :           skip ()       -> true");
    t.ok (n.skipN (),                 "      'de' :           skip ()       -> true");
    n.restore ();
    t.is (n.next (1), "b",            "    'bcde' :           skip ()       -> 'b'");
  }

  catch (std::string& e) {t.diag (e);}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
