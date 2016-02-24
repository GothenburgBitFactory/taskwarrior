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
#include <iostream>
#include <stdlib.h>
#include <main.h>
#include <text.h>
#include <utf8.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (180);

  // void wrapText (std::vector <std::string>& lines, const std::string& text, const int width, bool hyphenate)
  std::string text = "This is a test of the line wrapping code.";
  std::vector <std::string> lines;
  wrapText (lines, text, 10, true);
  t.is (lines.size (), (size_t) 5, "wrapText 'This is a test of the line wrapping code.' -> total 5 lines");
  t.is (lines[0], "This is a",     "wrapText line 0 -> 'This is a'");
  t.is (lines[1], "test of",       "wrapText line 1 -> 'test of'");
  t.is (lines[2], "the line",      "wrapText line 2 -> 'the line'");
  t.is (lines[3], "wrapping",      "wrapText line 3 -> 'wrapping'");
  t.is (lines[4], "code.",         "wrapText line 4 -> 'code.'");

  text = "This ☺ is a test of utf8 line extraction.";
  lines.clear ();
  wrapText (lines, text, 7, true);
  t.is (lines.size (), (size_t) 7, "wrapText 'This ☺ is a test of utf8 line extraction.' -> total 7 lines");
  t.is (lines[0], "This ☺",        "wrapText line 0 -> 'This ☺'");
  t.is (lines[1], "is a",          "wrapText line 1 -> 'is a'");
  t.is (lines[2], "test of",       "wrapText line 2 -> 'test of'");
  t.is (lines[3], "utf8",          "wrapText line 3 -> 'utf8'");
  t.is (lines[4], "line",          "wrapText line 4 -> 'line'");
  t.is (lines[5], "extrac-",       "wrapText line 5 -> 'extrac-'");
  t.is (lines[6], "tion.",         "wrapText line 6 -> 'tion.'");

  text = "one two three\n  four";
  lines.clear ();
  wrapText (lines, text, 13, true);
  t.is (lines.size (), (size_t) 2, "wrapText 'one two three\\n  four' -> 2 lines");
  t.is (lines[0], "one two three", "wrapText line 0 -> 'one two three'");
  t.is (lines[1], "  four",        "wrapText line 1 -> '  four'");

  // void extractLine (std::string& text, std::string& line, int length, bool hyphenate, unsigned int& offset)
  text = "This ☺ is a test of utf8 line extraction.";
  unsigned int offset = 0;
  std::string line;
  extractLine (line, text, 7, true, offset);
  t.is (line, "This ☺", "extractLine 7 'This ☺ is a test of utf8 line extraction.' -> 'This ☺'");

  // void extractLine (std::string& text, std::string& line, int length, bool hyphenate, unsigned int& offset)
  text = "line 1\nlengthy second line that exceeds width";
  offset = 0;
  extractLine (line, text, 10, true, offset);
  t.is (line, "line 1", "extractLine 10 'line 1\\nlengthy second line that exceeds width' -> 'line 1'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "lengthy", "extractLine 10 'lengthy second line that exceeds width' -> 'lengthy'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "second", "extractLine 10 'second line that exceeds width' -> 'second'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "line that", "extractLine 10 'line that exceeds width' -> 'line that'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "exceeds", "extractLine 10 'exceeds width' -> 'exceeds'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "width", "extractLine 10 'width' -> 'width'");

  t.notok (extractLine (line, text, 10, true, offset), "extractLine 10 '' -> ''");

  text = "AAAAAAAAAABBBBBBBBBB";
  offset = 0;
  extractLine (line, text, 10, true, offset);
  t.is (line, "AAAAAAAAA-", "extractLine hyphenated unbreakable line 'AAAAAAAAAABBBBBBBBBB'/10 -> 'AAAAAAAAA-'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "ABBBBBBBB-", "extractLine hyphenated unbreakable line 'AAAAAAAAAABBBBBBBBBB'/10 -> 'ABBBBBBBB-'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "BB", "extractLine hyphenated unbreakable line 'AAAAAAAAAABBBBBBBBBB'/10 -> 'BB'");

  text = "4444 333  ";
  offset = 0;
  while (extractLine (line, text, 9, true, offset))
    std::cout << "# line '" << line << "' offset " << offset << "\n";

  // void split (std::vector<std::string>& results, const std::string& input, const char delimiter)
  std::vector <std::string> items;
  std::string unsplit = "";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 0, "split '' '-' -> 0 items");

  unsplit = "a";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  unsplit = "-";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 2, "split '-' '-' -> '' ''");
  t.is (items[0], "",              "split '-' '-' -> [0] ''");
  t.is (items[1], "",              "split '-' '-' -> [1] ''");

  unsplit = "-a-bc-def";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 4, "split '-a-bc-def' '-' -> '' 'a' 'bc' 'def'");
  t.is (items[0], "",              "split '-a-bc-def' '-' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc-def' '-' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc-def' '-' -> [2] 'bc'");
  t.is (items[3], "def",           "split '-a-bc-def' '-' -> [3] 'def'");

  // void split (std::vector<std::string>& results, const std::string& input, const std::string& delimiter)
  unsplit = "";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 0, "split '' '--' -> 0 items");

  unsplit = "a";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 1, "split 'a' '--' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  unsplit = "--";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 2, "split '-' '--' -> '' ''");
  t.is (items[0], "",              "split '-' '-' -> [0] ''");
  t.is (items[1], "",              "split '-' '-' -> [1] ''");

  unsplit = "--a--bc--def";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 4, "split '-a-bc-def' '--' -> '' 'a' 'bc' 'def'");
  t.is (items[0], "",              "split '-a-bc-def' '--' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc-def' '--' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc-def' '--' -> [2] 'bc'");
  t.is (items[3], "def",           "split '-a-bc-def' '--' -> [3] 'def'");

  unsplit = "one\ntwo\nthree";
  split (items, unsplit, "\n");
  t.is (items.size (), (size_t) 3, "split 'one\\ntwo\\nthree' -> 'one', 'two', 'three'");
  t.is (items[0], "one",           "split 'one\\ntwo\\nthree' -> [0] 'one'");
  t.is (items[1], "two",           "split 'one\\ntwo\\nthree' -> [1] 'two'");
  t.is (items[2], "three",         "split 'one\\ntwo\\nthree' -> [2] 'three'");

  // void join (std::string& result, const std::string& separator, const std::vector<std::string>& items)
  std::vector <std::string> unjoined;
  std::string joined;

  join (joined, "", unjoined);
  t.is (joined.length (), (size_t) 0,  "join -> length 0");
  t.is (joined,           "",          "join -> ''");

  unjoined = {"", "a", "bc", "def"};
  join (joined, "", unjoined);
  t.is (joined.length (), (size_t) 6, "join '' 'a' 'bc' 'def' -> length 6");
  t.is (joined,           "abcdef",   "join '' 'a' 'bc' 'def' -> 'abcdef'");

  join (joined, "-", unjoined);
  t.is (joined.length (), (size_t) 9,  "join '' - 'a' - 'bc' - 'def' -> length 9");
  t.is (joined,           "-a-bc-def", "join '' - 'a' - 'bc' - 'def' -> '-a-bc-def'");

  // void join (std::string& result, const std::string& separator, const std::vector<int>& items)
  std::vector <int> unjoined2;

  join (joined, "", unjoined2);
  t.is (joined.length (), (size_t) 0, "join -> length 0");
  t.is (joined,           "",         "join -> ''");

  unjoined2 = {0, 1, 2};
  join (joined, "", unjoined2);
  t.is (joined.length (), (size_t) 3, "join 0 1 2 -> length 3");
  t.is (joined,           "012",      "join 0 1 2 -> '012'");

  join (joined, "-", unjoined2);
  t.is (joined.length (), (size_t) 5, "join 0 1 2 -> length 5");
  t.is (joined,           "0-1-2",    "join 0 1 2 -> '0-1-2'");

  // std::string unquoteText (const std::string& text)
  t.is (unquoteText (""),         "",     "unquoteText '' -> ''");
  t.is (unquoteText ("x"),        "x",    "unquoteText 'x' -> 'x'");
  t.is (unquoteText ("'x"),       "'x",   "unquoteText ''x' -> ''x'");
  t.is (unquoteText ("x'"),       "x'",   "unquoteText 'x'' -> 'x''");
  t.is (unquoteText ("\"x"),      "\"x",  "unquoteText '\"x' -> '\"x'");
  t.is (unquoteText ("x\""),      "x\"",  "unquoteText 'x\"' -> 'x\"'");
  t.is (unquoteText ("''"),       "",     "unquoteText '''' -> ''");
  t.is (unquoteText ("'''"),      "'",    "unquoteText ''''' -> '''");
  t.is (unquoteText ("\"\""),     "",     "unquoteText '\"\"' -> ''");
  t.is (unquoteText ("\"\"\""),    "\"",  "unquoteText '\"\"\"' -> '\"'");
  t.is (unquoteText ("''''"),     "''",   "unquoteText '''''' -> ''''");
  t.is (unquoteText ("\"\"\"\""), "\"\"", "unquoteText '\"\"\"\"' -> '\"\"'");
  t.is (unquoteText ("'\"\"'"),   "\"\"", "unquoteText '''\"\"' -> '\"\"'");
  t.is (unquoteText ("\"''\""),   "''",   "unquoteText '\"''\"' -> ''''");
  t.is (unquoteText ("'x'"),      "x",    "unquoteText ''x'' -> 'x'");
  t.is (unquoteText ("\"x\""),    "x",    "unquoteText '\"x\"' -> 'x'");

  // int longestWord (const std::string&)
  t.is (longestWord ("    "),                   0, "longestWord (    ) --> 0");
  t.is (longestWord ("this is a test"),         4, "longestWord (this is a test) --> 4");
  t.is (longestWord ("this is a better test"),  6, "longestWord (this is a better test) --> 6");
  t.is (longestWord ("house Çirçös clown"),     6, "longestWord (Çirçös) --> 6");

  // int longestLine (const std::string&)
  t.is (longestLine ("one two three four"),    18, "longestLine (one two three four) --> 18");
  t.is (longestLine ("one\ntwo three four"),   14, "longestLine (one\\ntwo three four) --> 14");
  t.is (longestLine ("one\ntwo\nthree\nfour"),  5, "longestLine (one\\ntwo\\nthree\\nfour) --> 5");

  // bool nontrivial (const std::string&);
  t.notok (nontrivial (""),                       "nontrivial '' -> false");
  t.notok (nontrivial ("   "),                    "nontrivial '   ' -> false");
  t.notok (nontrivial ("\t\t"),                   "nontrivial '\\t\\t' -> false");
  t.notok (nontrivial (" \t \t"),                 "nontrivial ' \\t \\t' -> false");
  t.ok    (nontrivial ("a"),                      "nontrivial 'a' -> true");
  t.ok    (nontrivial ("   a"),                   "nontrivial '   a' -> true");
  t.ok    (nontrivial ("a   "),                   "nontrivial 'a   ' -> true");
  t.ok    (nontrivial ("  \t\ta"),                "nontrivial '  \\t\\ta' -> true");
  t.ok    (nontrivial ("a\t\t  "),                "nontrivial 'a\\t\\t  ' -> true");

  // bool compare (const std::string&, const std::string&, bool caseless = false);
  // Make sure degenerate cases are handled.
  t.ok    (compare ("", ""),    "'' == ''");
  t.notok (compare ("foo", ""), "foo != ''");
  t.notok (compare ("", "foo"), "'' != foo");

  // Make sure the default is case-sensitive.
  t.ok    (compare ("foo", "foo"), "foo == foo");
  t.notok (compare ("foo", "FOO"), "foo != foo");

  // Test case-sensitive.
  t.notok (compare ("foo", "xx", true),  "foo != xx");

  t.ok    (compare ("foo", "foo", true), "foo == foo");
  t.notok (compare ("foo", "FOO", true), "foo != FOO");
  t.notok (compare ("FOO", "foo", true), "FOO != foo");
  t.ok    (compare ("FOO", "FOO", true), "FOO == FOO");

  // Test case-insensitive.
  t.notok (compare ("foo", "xx", false),   "foo != foo (caseless)");

  t.ok    (compare ("foo", "foo", false),  "foo == foo (caseless)");
  t.ok    (compare ("foo", "FOO", false),  "foo == FOO (caseless)");
  t.ok    (compare ("FOO", "foo", false),  "FOO == foo (caseless)");
  t.ok    (compare ("FOO", "FOO", false),  "FOO == FOO (caseless)");

  // std::string::size_type find (const std::string&, const std::string&, bool caseless = false);
  // Make sure degenerate cases are handled.
  t.is ((int) find ("foo", ""), (int) 0,                 "foo !contains ''");
  t.is ((int) find ("", "foo"), (int) std::string::npos, "'' !contains foo");

  // Make sure the default is case-sensitive.
  t.is ((int) find ("foo", "fo"), 0,                       "foo contains fo");
  t.is ((int) find ("foo", "FO"), (int) std::string::npos, "foo !contains fo");

  // Test case-sensitive.
  t.is ((int) find ("foo", "xx", true), (int) std::string::npos, "foo !contains xx");
  t.is ((int) find ("foo", "oo", true), 1,                       "foo contains oo");

  t.is ((int) find ("foo", "fo", true), 0,                       "foo contains fo");
  t.is ((int) find ("foo", "FO", true), (int) std::string::npos, "foo !contains fo");
  t.is ((int) find ("FOO", "fo", true), (int) std::string::npos, "foo !contains fo");
  t.is ((int) find ("FOO", "FO", true), 0,                       "foo contains fo");

  // Test case-insensitive.
  t.is ((int) find ("foo", "xx", false),  (int) std::string::npos, "foo !contains xx (caseless)");
  t.is ((int) find ("foo", "oo", false),  1,                       "foo contains oo (caseless)");

  t.is ((int) find ("foo", "fo", false),  0, "foo contains fo (caseless)");
  t.is ((int) find ("foo", "FO", false),  0, "foo contains FO (caseless)");
  t.is ((int) find ("FOO", "fo", false),  0, "FOO contains fo (caseless)");
  t.is ((int) find ("FOO", "FO", false),  0, "FOO contains FO (caseless)");

  // Test start offset.
  t.is ((int) find ("one two three", "e",  3, true), (int) 11, "offset obeyed");
  t.is ((int) find ("one two three", "e", 11, true), (int) 11, "offset obeyed");

  t.is ((int) find ("one two three", "e",  3, false), (int) 11, "offset obeyed");
  t.is ((int) find ("one two three", "e", 11, false), (int) 11, "offset obeyed");

  // int strippedLength (const std::string&);
  t.is (strippedLength (std::string ("")),                                  0, "strippedLength                              -> 0");
  t.is (strippedLength (std::string ("abc")),                               3, "strippedLength abc                          -> 3");
  t.is (strippedLength (std::string ("one\033[5;38;255mtwo\033[0mthree")), 11, "strippedLength one^[[5;38;255mtwo^[[0mthree -> 11");
  t.is (strippedLength (std::string ("\033[0m")),                           0, "strippedLength ^[[0m                        -> 0");
  t.is (strippedLength (std::string ("\033[1m\033[0m")),                    0, "strippedLength ^[[1m^[[0m                   -> 0");

  // std::string format (char);
  t.is (format ('A'), "A", "format ('A') -> A");

  // std::string format (int);
  t.is (format (0),  "0",  "format (0) -> 0");
  t.is (format (-1), "-1", "format (-1) -> -1");

  // std::string formatHex (int);
  t.is (formatHex (0),   "0",  "formatHex (0) -> 0");
  t.is (formatHex (10),  "a",  "formatHex (10) -> a");
  t.is (formatHex (123), "7b", "formatHex (123) -> 7b");

  // std::string format (float, int, int);
  t.is (format (0.12345678, 8, 4),      "  0.1235",     "format (0.12345678,    8,   4) -> __0.1235");

  t.is (format (1.23456789, 8, 1),      "       1",     "format (1.23456789,    8,   1) -> _______1");
  t.is (format (1.23456789, 8, 2),      "     1.2",     "format (1.23456789,    8,   2) -> _____1.2");
  t.is (format (1.23456789, 8, 3),      "    1.23",     "format (1.23456789,    8,   3) -> ____1.23");
  t.is (format (1.23456789, 8, 4),      "   1.235",     "format (1.23456789,    8,   4) -> ___1.235");
  t.is (format (1.23456789, 8, 5),      "  1.2346",     "format (1.23456789,    8,   5) -> __1.2346");
  t.is (format (1.23456789, 8, 6),      " 1.23457",     "format (1.23456789,    8,   6) ->  1.23457");
  t.is (format (1.23456789, 8, 7),      "1.234568",     "format (1.23456789,    8,   7) -> 1.234568");
  t.is (format (1.23456789, 8, 8),      "1.2345679",    "format (1.23456789,    8,   8) -> 1.2345679");
  t.is (format (2444238.56789, 12, 11), "2444238.5679", "format (2444238.56789, 12, 11) -> 2444238.5679");

  // std::string format (double, int, int);

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify (123, 3), "123",   "leftJustify 123,3 -> '123'");
  t.is (leftJustify (123, 4), "123 ",  "leftJustify 123,4 -> '123 '");
  t.is (leftJustify (123, 5), "123  ", "leftJustify 123,5 -> '123  '");

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify ("foo", 3), "foo",   "leftJustify foo,3 -> 'foo'");
  t.is (leftJustify ("foo", 4), "foo ",  "leftJustify foo,4 -> 'foo '");
  t.is (leftJustify ("foo", 5), "foo  ", "leftJustify foo,5 -> 'foo  '");
  t.is (leftJustify ("föo", 5), "föo  ", "leftJustify föo,5 -> 'föo  '");

  // When a string doesn't fit in the space necessary‥
  t.is (leftJustify ("foo", 2), "fo",    "leftJustify foo,2 -→ 'fo'");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify (123, 3), "123",   "rightJustify 123,3 -> '123'");
  t.is (rightJustify (123, 4), " 123",  "rightJustify 123,4 -> ' 123'");
  t.is (rightJustify (123, 5), "  123", "rightJustify 123,5 -> '  123'");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify ("foo", 3), "foo",   "rightJustify foo,3 -> 'foo'");
  t.is (rightJustify ("foo", 4), " foo",  "rightJustify foo,4 -> ' foo'");
  t.is (rightJustify ("foo", 5), "  foo", "rightJustify foo,5 -> '  foo'");
  t.is (rightJustify ("föo", 5), "  föo", "rightJustify föo,5 -> '  föo'");

  // bool closeEnough (const std::string&, const std::string&, unsigned int minLength = 0);
  t.ok (closeEnough ("foobar", "foobar"),      "closeEnough foobar == foobar");
  t.ok (closeEnough ("foobar", "foobar", 0),   "closeEnough foobar == foobar,0");
  t.ok (closeEnough ("foobar", "foobar", 1),   "closeEnough foobar == foobar,1");
  t.ok (closeEnough ("foobar", "foobar", 2),   "closeEnough foobar == foobar,2");
  t.ok (closeEnough ("foobar", "foobar", 3),   "closeEnough foobar == foobar,3");
  t.ok (closeEnough ("foobar", "foobar", 4),   "closeEnough foobar == foobar,4");
  t.ok (closeEnough ("foobar", "foobar", 5),   "closeEnough foobar == foobar,5");
  t.ok (closeEnough ("foobar", "foobar", 6),   "closeEnough foobar == foobar,6");
  t.ok (closeEnough ("foobar", "foo", 3),      "closeEnough foobar == foo,3");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
