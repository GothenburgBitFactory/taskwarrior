////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <stdlib.h>
#include <Context.h>
#include <Nibbler.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (234);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  try
  {
    Nibbler n;
    std::string s;
    int i;
    double d;
    std::vector <std::string> options;

    // Make sure the nibbler behaves itself with trivial input.
    t.diag ("Test all nibbler calls given empty input");
    n = Nibbler ("");
    t.notok (n.getUntil (' ', s),        "trivial: getUntil");
    t.notok (n.getUntil ("hi", s),       "trivial: getUntil");
    t.notok (n.getUntilOneOf ("ab", s),  "trivial: getUntilOneOf");
    t.notok (n.skipN (123),              "trivial: skipN");
    t.notok (n.skip ('x'),               "trivial: skip");
    t.notok (n.skipAllOneOf ("abc"),     "trivial: skipAllOneOf");
    t.notok (n.getQuoted ('"', s),       "trivial: getQuoted");
    t.notok (n.getDigit (i),             "trivial: getDigit");
    t.notok (n.getInt (i),               "trivial: getInt"); // 10
    t.notok (n.getUnsignedInt (i),       "trivial: getUnsignedInt");
    t.notok (n.getUntilEOS (s),          "trivial: getUntilEOS");
    t.notok (n.getOneOf (options, s),    "trivial: getOneOf");
    t.ok    (n.depleted (),              "trivial: depleted");

    // bool getUntil (char, std::string&);
    t.diag ("Nibbler::getUntil");
    n = Nibbler ("one two");
    t.ok    (n.getUntil (' ', s),        " 'one two' :       getUntil (' ')    -> true");
    t.is    (s, "one",                   " 'one two' :       getUntil (' ')    -> 'one'"); // 20
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
    t.notok (n.getQuoted ('\'', s),   "       'x' :      getQuoted (''')    -> false"); // 90

    n = Nibbler ("\"one\\\"two\"");
    t.notok (n.getQuoted ('\'', s),   "\"one\\\"two\" :      getQuoted (''')    -> false");

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s), "\"one\\\"two\" :      getQuoted ('\"', s) -> true");
    t.is (s, "one\\\"two", "getQuoted ('\"', s) -> one\\\"two");

    n = Nibbler ("\"one\\\"two\"");
    t.ok (n.getQuoted ('"', s), "\"one\\\"two\" :      getQuoted ('\"', s) -> true");
    t.is (s, "one\\\"two", "getQuoted ('\"', s) -> one\\\"two");

    n = Nibbler ("\"one\\\\\"");
    t.ok (n.getQuoted ('\"', s), "one\\ :              getQuoted ('\"', s)      -> true");
    t.is (s, "one\\\\",                                       "getQuoted ('\"', s)      -> \"one\\\\\"");

    // bool getDigit (int&);
    t.diag ("Nibbler::getDigit");
    n = Nibbler ("12x");
    t.ok    (n.getDigit (i),          "     '12x' :         getDigit ()     -> true");
    t.is    (i, 1,                    "     '12x' :         getDigit ()     -> 1");
    t.ok    (n.getDigit (i),          "      '2x' :         getDigit ()     -> true");
    t.is    (i, 2,                    "      '2x' :         getDigit ()     -> 2");
    t.notok (n.getDigit (i),          "       'x' :         getDigit ()     -> false");

    // bool getDigit4 (int&);
    t.diag ("Nibbler::getDigit4");
    n = Nibbler ("4321");
    t.ok    (n.getDigit4 (i),         "      4321 :         getDigit4 ()    -> true");
    t.is    (i, 4321,                 "      4321 :         getDigit4 ()    -> 4321");

    // bool getDigit2 (int&);
    t.diag ("Nibbler::getDigit2");
    n = Nibbler ("21");
    t.ok    (n.getDigit2 (i),         "        21 :         getDigit2 ()    -> true");
    t.is    (i, 21,                   "        21 :         getDigit2 ()    -> 21");

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

    // bool getNumber (double&);
    t.diag ("Nibbler::getNumber");
    n = Nibbler ("-1.234 2.3e4");
    t.ok    (n.getNumber (d),         "'-1.234 2.3e4' : getNumber ()       -> true");
    t.is    (d, -1.234, 0.000001,     "'-1.234 2.3e4' : getNumber ()       -> '-1.234'");
    t.ok    (n.skip (' '),            "      ' 2.3e4' : skip (' ')         -> true");
    t.ok    (n.getNumber (d),         "       '2.3e4' : getNumber ()       -> true");
    t.is    (d, 2.3e4,                "       '2.3e4' : getNumber ()       -> '2.3e4'");
    t.ok    (n.depleted (),           "            '' : depleted ()        -> true");

    n = Nibbler ("2.0");
    t.ok    (n.getNumber (d),         "'2.0' : getNumber ()                -> true");
    t.is    (d, 2.0, 0.000001,        "'2.0' : getNumber ()                -> '2.0'");
    t.ok    (n.depleted (),           "            '' : depleted ()        -> true");

    n = Nibbler ("-864000.00000");
    t.ok    (n.getNumber (d),         "'-864000.00000' : getNumber ()      -> true");
    t.is    (d, -864000.0,            "'-864000.00000' : getNumber ()      -> -864000.0");
    t.ok    (n.depleted (),           "             '' : depleted ()       -> true");

    // bool getLiteral (const std::string&);
    t.diag ("Nibbler::getLiteral");
    n = Nibbler ("foobar");
    t.ok    (n.getLiteral ("foo"),    "  'foobar' :     getLiteral ('foo')  -> true");
    t.notok (n.getLiteral ("foo"),    "     'bar' :     getLiteral ('foo')  -> false");
    t.ok    (n.getLiteral ("bar"),    "     'bar' :     getLiteral ('bar')  -> true");
    t.ok    (n.depleted (),           "        '' :       depleted ()       -> true");

    // bool getPartialUUID (std::string&);
    t.diag ("Nibbler::getPartialUUID");
    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4d5");
    t.ok (n.getPartialUUID (s),                      "partial uuid [36] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4d5", "partial uuid [36] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4d");
    t.ok (n.getPartialUUID (s),                      "partial uuid [35] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4d",  "partial uuid [35] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4");
    t.ok (n.getPartialUUID (s),                      "partial uuid [34] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c4",   "partial uuid [34] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c");
    t.ok (n.getPartialUUID (s),                      "partial uuid [33] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3c",    "partial uuid [33] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3");
    t.ok (n.getPartialUUID (s),                      "partial uuid [32] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b3",     "partial uuid [32] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b");
    t.ok (n.getPartialUUID (s),                      "partial uuid [31] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2b",      "partial uuid [31] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2");
    t.ok (n.getPartialUUID (s),                      "partial uuid [30] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a2",       "partial uuid [30] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1a");
    t.ok (n.getPartialUUID (s),                      "partial uuid [29] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1a",        "partial uuid [29] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F1");
    t.ok (n.getPartialUUID (s),                      "partial uuid [28] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F1",         "partial uuid [28] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0F");
    t.ok (n.getPartialUUID (s),                      "partial uuid [27] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0F",          "partial uuid [27] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E0");
    t.ok (n.getPartialUUID (s),                      "partial uuid [26] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E0",           "partial uuid [26] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-E");
    t.ok (n.getPartialUUID (s),                      "partial uuid [25] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-E",            "partial uuid [25] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9-");
    t.ok (n.getPartialUUID (s),                      "partial uuid [24] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9-",             "partial uuid [24] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D9");
    t.ok (n.getPartialUUID (s),                      "partial uuid [23] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D9",              "partial uuid [23] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8D");
    t.ok (n.getPartialUUID (s),                      "partial uuid [22] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8D",               "partial uuid [22] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C8");
    t.ok (n.getPartialUUID (s),                      "partial uuid [21] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C8",                "partial uuid [21] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-C");
    t.ok (n.getPartialUUID (s),                      "partial uuid [20] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-C",                 "partial uuid [20] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7-");
    t.ok (n.getPartialUUID (s),                      "partial uuid [19] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7-",                  "partial uuid [19] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B7");
    t.ok (n.getPartialUUID (s),                      "partial uuid [18] found");
    t.is (s, "a0b1c2d3-e4f5-A6B7",                   "partial uuid [18] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6B");
    t.ok (n.getPartialUUID (s),                      "partial uuid [17] found");
    t.is (s, "a0b1c2d3-e4f5-A6B",                    "partial uuid [17] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A6");
    t.ok (n.getPartialUUID (s),                      "partial uuid [16] found");
    t.is (s, "a0b1c2d3-e4f5-A6",                     "partial uuid [16] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-A");
    t.ok (n.getPartialUUID (s),                      "partial uuid [15] found");
    t.is (s, "a0b1c2d3-e4f5-A",                      "partial uuid [15] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5-");
    t.ok (n.getPartialUUID (s),                      "partial uuid [14] found");
    t.is (s, "a0b1c2d3-e4f5-",                       "partial uuid [14] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f5");
    t.ok (n.getPartialUUID (s),                      "partial uuid [13] found");
    t.is (s, "a0b1c2d3-e4f5",                        "partial uuid [13] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4f");
    t.ok (n.getPartialUUID (s),                      "partial uuid [12] found");
    t.is (s, "a0b1c2d3-e4f",                         "partial uuid [12] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e4");
    t.ok (n.getPartialUUID (s),                      "partial uuid [11] found");
    t.is (s, "a0b1c2d3-e4",                          "partial uuid [11] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-e");
    t.ok (n.getPartialUUID (s),                      "partial uuid [10] found");
    t.is (s, "a0b1c2d3-e",                           "partial uuid [10] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3-");
    t.ok (n.getPartialUUID (s),                      "partial uuid [9] found");
    t.is (s, "a0b1c2d3-",                            "partial uuid [9] -> correct");
    t.ok (n.depleted (),                             "depleted");

    n = Nibbler ("a0b1c2d3");
    t.ok (n.getPartialUUID (s),                      "partial uuid [8] found");
    t.is (s, "a0b1c2d3",                             "partial uuid [8] -> correct");
    t.ok (n.depleted (),                             "not depleted");

    // bool getOneOf (const std::vector <std::string>&, std::string&);
    t.diag ("Nibbler::getOneOf");
    options = {"one", "two", "three"};
    n = Nibbler ("onetwothreefour");
    t.ok    (n.getOneOf (options, s),         "'onetwothreefour':   getOneOf () -> true");
    t.is    (s, "one",                        "'onetwothreefour':   getOneOf () -> one");
    t.ok    (n.getOneOf (options, s),         "   'twothreefour':   getOneOf () -> true");
    t.is    (s, "two",                        "   'twothreefour':   getOneOf () -> two");
    t.ok    (n.getOneOf (options, s),         "      'threefour':   getOneOf () -> true");
    t.is    (s, "three",                      "      'threefour':   getOneOf () -> three");
    t.notok (n.getOneOf (options, s),         "           'four':   getOneOf () -> false");

    // bool getN (int, std::string&);
    t.diag ("Nibbler::getN");
    n = Nibbler ("111223");
    t.ok (n.getN (3, s),              "  '111223' : getN (3)         -> true");
    t.is (s, "111",                   "  '111223' : getN (3)         -> '111'");
    t.ok (n.getN (2, s),              "     '223' : getN (2)         -> true");
    t.is (s, "22",                    "     '223' : getN (2)         -> '22'");
    t.ok (n.getN (1, s),              "       '3' : getN (1)         -> true");
    t.is (s, "3",                     "       '3' : getN (1)         -> '1'");
    t.ok    (n.depleted (),           "        '' : depleted ()      -> true");

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

  catch (const std::string& e) {t.diag (e);}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
