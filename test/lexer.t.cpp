////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <vector>
#include <string.h>
#include <test.h>
#include <Lexer.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
#ifdef PRODUCT_TASKWARRIOR
  UnitTest t (1280);
#else
  UnitTest t (1262);
#endif

  std::vector <std::pair <std::string, Lexer::Type>> tokens;
  std::string token;
  Lexer::Type type;

  // Feed in some attributes and types, so that the Lexer knows what a DOM
  // reference is.
  Lexer::attributes["due"]         = "date";
  Lexer::attributes["tags"]        = "string";
  Lexer::attributes["description"] = "string";

  // White space detection.
  t.notok (Lexer::isWhitespace (0x0041), "U+0041 (A) ! isWhitespace");
  t.ok (Lexer::isWhitespace (0x0020), "U+0020 isWhitespace");
  t.ok (Lexer::isWhitespace (0x0009), "U+0009 isWhitespace");
  t.ok (Lexer::isWhitespace (0x000A), "U+000A isWhitespace");
  t.ok (Lexer::isWhitespace (0x000B), "U+000B isWhitespace");
  t.ok (Lexer::isWhitespace (0x000C), "U+000C isWhitespace");
  t.ok (Lexer::isWhitespace (0x000D), "U+000D isWhitespace");
  t.ok (Lexer::isWhitespace (0x0085), "U+0085 isWhitespace");
  t.ok (Lexer::isWhitespace (0x00A0), "U+00A0 isWhitespace");
  t.ok (Lexer::isWhitespace (0x1680), "U+1680 isWhitespace"); // 10
  t.ok (Lexer::isWhitespace (0x180E), "U+180E isWhitespace");
  t.ok (Lexer::isWhitespace (0x2000), "U+2000 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2001), "U+2001 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2002), "U+2002 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2003), "U+2003 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2004), "U+2004 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2005), "U+2005 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2006), "U+2006 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2007), "U+2007 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2008), "U+2008 isWhitespace"); // 20
  t.ok (Lexer::isWhitespace (0x2009), "U+2009 isWhitespace");
  t.ok (Lexer::isWhitespace (0x200A), "U+200A isWhitespace");
  t.ok (Lexer::isWhitespace (0x2028), "U+2028 isWhitespace");
  t.ok (Lexer::isWhitespace (0x2029), "U+2029 isWhitespace");
  t.ok (Lexer::isWhitespace (0x202F), "U+202F isWhitespace");
  t.ok (Lexer::isWhitespace (0x205F), "U+205F isWhitespace");
  t.ok (Lexer::isWhitespace (0x3000), "U+3000 isWhitespace");

  // static bool Lexer::isBoundary (int, int);
  t.ok    (Lexer::isBoundary (' ', 'a'), "' ' --> 'a' = isBoundary");
  t.ok    (Lexer::isBoundary ('a', ' '), "'a' --> ' ' = isBoundary");
  t.ok    (Lexer::isBoundary (' ', '+'), "' ' --> '+' = isBoundary");
  t.ok    (Lexer::isBoundary (' ', ','), "' ' --> ',' = isBoundary");
  t.notok (Lexer::isBoundary ('3', '4'), "'3' --> '4' = isBoundary");
  t.ok    (Lexer::isBoundary ('(', '('), "'(' --> '(' = isBoundary");
  t.notok (Lexer::isBoundary ('r', 'd'), "'r' --> 'd' = isBoundary");

  // static bool Lexer::wasQuoted (const std::string&);
  t.notok (Lexer::wasQuoted (""),        "'' --> !wasQuoted");
  t.notok (Lexer::wasQuoted ("foo"),     "'foo' --> !wasQuoted");
  t.ok    (Lexer::wasQuoted ("a b"),     "'a b' --> wasQuoted");
  t.ok    (Lexer::wasQuoted ("(a)"),     "'(a)' --> wasQuoted");

  // static bool Lexer::dequote (std::string&, const std::string& quotes = "'\"");
  token = "foo";
  Lexer::dequote (token);
  t.is (token, "foo", "dequote foo --> foo");

  token = "'foo'";
  Lexer::dequote (token);
  t.is (token, "foo", "dequote 'foo' --> foo");

  token = "'o\\'clock'";
  Lexer::dequote (token);
  t.is (token, "o\\'clock", "dequote 'o\\'clock' --> o\\'clock");

  token = "abba";
  Lexer::dequote (token, "a");
  t.is (token, "bb", "dequote 'abba' (a) --> bb");

  // Should result in no tokens.
  Lexer l0 ("");
  t.notok (l0.token (token, type), "'' --> no tokens");

  // Should result in no tokens.
  Lexer l1 ("       \t ");
  t.notok (l1.token (token, type), "'       \\t ' --> no tokens");

  // \u20ac = Euro symbol.
  Lexer l2 (" one 'two \\'three\\''+456-(1.3*2 - 0x12) 1.2e-3.4    foo.bar and '\\u20ac'");

  tokens.clear ();
  while (l2.token (token, type))
  {
    std::cout << "# «" << token << "» " << Lexer::typeName (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is (tokens[0].first,                     "one",           "tokens[0] = 'one'"); // 30
  t.is (Lexer::typeName (tokens[0].second),  "identifier",    "tokens[0] = identifier");
  t.is (tokens[1].first,                     "'two 'three''", "tokens[1] = 'two 'three''");
  t.is (Lexer::typeName (tokens[1].second),  "string",        "tokens[1] = string");
  t.is (tokens[2].first,                     "+",             "tokens[2] = '+'");
  t.is (Lexer::typeName (tokens[2].second),  "op",            "tokens[2] = op");
  t.is (tokens[3].first,                     "456",           "tokens[3] = '456'");
  t.is (Lexer::typeName (tokens[3].second),  "number",        "tokens[3] = number");
  t.is (tokens[4].first,                     "-",             "tokens[4] = '-'");
  t.is (Lexer::typeName (tokens[4].second),  "op",            "tokens[4] = op");
  t.is (tokens[5].first,                     "(",             "tokens[5] = '('"); // 40
  t.is (Lexer::typeName (tokens[5].second),  "op",            "tokens[5] = op");
  t.is (tokens[6].first,                     "1.3",           "tokens[6] = '1.3'");
  t.is (Lexer::typeName (tokens[6].second),  "number",        "tokens[6] = number");
  t.is (tokens[7].first,                     "*",             "tokens[7] = '*'");
  t.is (Lexer::typeName (tokens[7].second),  "op",            "tokens[7] = op");
  t.is (tokens[8].first,                     "2",             "tokens[8] = '2'");
  t.is (Lexer::typeName (tokens[8].second),  "number",        "tokens[8] = number");
  t.is (tokens[9].first,                     "-",             "tokens[9] = '-'");
  t.is (Lexer::typeName (tokens[9].second),  "op",            "tokens[9] = op");
  t.is (tokens[10].first,                    "0x12",          "tokens[10] = '0x12'"); // 50
  t.is (Lexer::typeName (tokens[10].second), "hex",           "tokens[10] = hex");
  t.is (tokens[11].first,                    ")",             "tokens[11] = ')'");
  t.is (Lexer::typeName (tokens[11].second), "op",            "tokens[11] = op");
  t.is (tokens[12].first,                    "1.2e-3.4",      "tokens[12] = '1.2e-3.4'");
  t.is (Lexer::typeName (tokens[12].second), "number",        "tokens[12] = number");
  t.is (tokens[13].first,                    "foo.bar",       "tokens[13] = 'foo.bar'");
  t.is (Lexer::typeName (tokens[13].second), "identifier",    "tokens[13] = identifier");
  t.is (tokens[14].first,                    "and",           "tokens[14] = 'and'"); // 60
  t.is (Lexer::typeName (tokens[14].second), "op",            "tokens[14] = op");
  t.is (tokens[15].first,                    "'€'",           "tokens[15] = \\u20ac --> ''€''");
  t.is (Lexer::typeName (tokens[15].second), "string",        "tokens[15] = string");

  // Test for numbers that are no longer ISO-8601 dates.
  Lexer l3 ("1 12 123 1234 12345 123456 1234567");
  tokens.clear ();
  while (l3.token (token, type))
  {
    std::cout << "# «" << token << "» " << Lexer::typeName (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),     7,                         "7 tokens");
  t.is (tokens[0].first,         "1",                       "tokens[0] == '1'");
  t.is ((int) tokens[0].second,  (int) Lexer::Type::number, "tokens[0] == Type::number");
  t.is (tokens[1].first,         "12",                      "tokens[1] == '12'");
  t.is ((int) tokens[1].second,  (int) Lexer::Type::number, "tokens[1] == Type::date");
  t.is (tokens[2].first,         "123",                     "tokens[2] == '123'");
  t.is ((int) tokens[2].second,  (int) Lexer::Type::number, "tokens[2] == Type::number"); // 70
  t.is (tokens[3].first,         "1234",                    "tokens[3] == '1234'");
  t.is ((int) tokens[3].second,  (int) Lexer::Type::number, "tokens[3] == Type::date");
  t.is (tokens[4].first,         "12345",                   "tokens[4] == '12345'");
  t.is ((int) tokens[4].second,  (int) Lexer::Type::number, "tokens[4] == Type::number");
  t.is (tokens[5].first,         "123456",                  "tokens[5] == '123456'");
  t.is ((int) tokens[5].second,  (int) Lexer::Type::number, "tokens[5] == Type::date");
  t.is (tokens[6].first,         "1234567",                 "tokens[6] == '1234567'");
  t.is ((int) tokens[6].second,  (int) Lexer::Type::number, "tokens[6] == Type::number");

  // void split (std::vector<std::string>&, const std::string&);
  std::string unsplit = " ( A or B ) ";
  std::vector <std::string> items;
  items = Lexer::split (unsplit);
  t.is (items.size (), (size_t) 5, "split ' ( A or B ) '");
  t.is (items[0], "(",             "split ' ( A or B ) ' -> [0] '('");
  t.is (items[1], "A",             "split ' ( A or B ) ' -> [1] 'A'");
  t.is (items[2], "or",            "split ' ( A or B ) ' -> [2] 'or'");
  t.is (items[3], "B",             "split ' ( A or B ) ' -> [3] 'B'");
  t.is (items[4], ")",             "split ' ( A or B ) ' -> [4] ')'");

  // Test simple mode with contrived tokens that ordinarily split.
  unsplit = "  +-* a+b 12.3e4 'c d'";
  items = Lexer::split (unsplit);
  t.is (items.size (), (size_t) 8, "split '  +-* a+b 12.3e4 'c d''");
  t.is (items[0], "+",             "split '  +-* a+b 12.3e4 'c d'' -> [0] '+'");
  t.is (items[1], "-",             "split '  +-* a+b 12.3e4 'c d'' -> [1] '-'");
  t.is (items[2], "*",             "split '  +-* a+b 12.3e4 'c d'' -> [2] '*'");
  t.is (items[3], "a",             "split '  +-* a+b 12.3e4 'c d'' -> [3] 'a'");
  t.is (items[4], "+",             "split '  +-* a+b 12.3e4 'c d'' -> [4] '+'");
  t.is (items[5], "b",             "split '  +-* a+b 12.3e4 'c d'' -> [5] 'b'");
  t.is (items[6], "12.3e4",        "split '  +-* a+b 12.3e4 'c d'' -> [6] '12.3e4'");
  t.is (items[7], "'c d'",         "split '  +-* a+b 12.3e4 'c d'' -> [7] ''c d''");

  // static bool decomposePair (const std::string&, std::string&, std::string&, std::string&, std::string&);
  // 2 * 4 * 2 * 5 = 80 tests.
  std::string outName, outMod, outValue, outSep;
  for (auto& name : {"name"})
  {
    for (auto& mod : {"", "mod"})
    {
      for (auto& sep : {":", "=", "::", ":="})
      {
        for (auto& value : {"", "value", "a:b", "a::b", "a=b", "a:=b"})
        {
          std::string input = std::string ("name") + (strlen (mod) ? "." : "") + mod + sep + value;
          t.ok (Lexer::decomposePair (input, outName, outMod, outSep, outValue), "decomposePair '" + input + "' --> true");
          t.is (name,  outName,  "  '" + input + "' --> name '"  + name  + "'");
          t.is (mod,   outMod,   "  '" + input + "' --> mod '"   + mod   + "'");
          t.is (value, outValue, "  '" + input + "' --> value '" + value + "'");
          t.is (sep,   outSep,   "  '" + input + "' --> sep '"   + sep   + "'");
        }
      }
    }
  }

  // static bool readWord (const std::string&, const std::string&, std::string::size_type&, std::string&);
  std::string::size_type cursor = 0;
  std::string word;
  t.ok (Lexer::readWord ("'one two'", "'\"", cursor, word), "readWord ''one two'' --> true");
  t.is (word, "'one two'",                                  "  word '" + word + "'");
  t.is ((int)cursor, 9,                                     "  cursor");

  // Unterminated quoted string is invalid.
  cursor = 0;
  t.notok (Lexer::readWord ("'one", "'\"", cursor, word),   "readWord ''one' --> false");

  // static bool readWord (const std::string&, std::string::size_type&, std::string&);
  cursor = 0;
  t.ok (Lexer::readWord ("input", cursor, word),            "readWord 'input' --> true");
  t.is (word, "input",                                      "  word '" + word + "'");
  t.is ((int)cursor, 5,                                     "  cursor");

  cursor = 0;
  t.ok (Lexer::readWord ("one\\ two", cursor, word),        "readWord 'one\\ two' --> true");
  t.is (word, "one two",                                    "  word '" + word + "'");
  t.is ((int)cursor, 8,                                     "  cursor");

  cursor = 0;
  t.ok (Lexer::readWord ("\\u20A43", cursor, word),         "readWord '\\u20A43' --> true");
  t.is (word, "₤3",                                         "  word '" + word + "'");
  t.is ((int)cursor, 7,                                     "  cursor");

  cursor = 0;
  t.ok (Lexer::readWord ("U+20AC4", cursor, word),          "readWord '\\u20AC4' --> true");
  t.is (word, "€4",                                         "  word '" + word + "'");
  t.is ((int)cursor, 7,                                     "  cursor");

  std::string text = "one 'two' three\\ four";
  cursor = 0;
  t.ok (Lexer::readWord (text, cursor, word),               "readWord \"one 'two' three\\ four\" --> true");
  t.is (word, "one",                                        "  word '" + word + "'");
  cursor++;
  t.ok (Lexer::readWord (text, cursor, word),               "readWord \"one 'two' three\\ four\" --> true");
  t.is (word, "'two'",                                      "  word '" + word + "'");
  cursor++;
  t.ok (Lexer::readWord (text, cursor, word),               "readWord \"one 'two' three\\ four\" --> true");
  t.is (word, "three four",                                 "  word '" + word + "'");

  text = "one     ";
  cursor = 0;
  t.ok (Lexer::readWord (text, cursor, word),               "readWord \"one     \" --> true");
  t.is (word, "one",                                        "  word '" + word + "'");

  // bool isLiteral (const std::string&, bool, bool);
  Lexer l4 ("one.two");
  t.notok (l4.isLiteral("zero", false, false),              "isLiteral 'one.two' --> false");
  t.ok    (l4.isLiteral("one",  false, false),              "isLiteral 'one.two' --> 'one'");
  t.ok    (l4.isLiteral(".",    false, false),              "isLiteral 'one.two' --> '.'");
  t.ok    (l4.isLiteral("two",  false, true),               "isLiteral 'one.two' --> 'two'");

  Lexer l5 ("wonder");
  t.notok (l5.isLiteral ("wonderful", false, false),        "isLiteral 'wonderful' != 'wonder' without abbreviation");
  t.ok    (l5.isLiteral ("wonderful", true,  false),        "isLiteral 'wonderful' == 'wonder' with abbreviation");

  // bool isOneOf (const std::string&, bool, bool);
  Lexer l6 ("Grumpy.");
  std::vector <std::string> dwarves = {"Sneezy", "Doc", "Bashful", "Grumpy", "Happy", "Sleepy", "Dopey"};
  t.notok (l6.isOneOf (dwarves, false, true),               "isOneof ('Grumpy', true) --> false");
  t.ok    (l6.isOneOf (dwarves, false, false),              "isOneOf ('Grumpy', false) --> true");

  // static std::string::size_type commonLength (const std::string&, const std::string&);
  t.is ((int)Lexer::commonLength ("", ""),           0, "commonLength '' : '' --> 0");
  t.is ((int)Lexer::commonLength ("a", "a"),         1, "commonLength 'a' : 'a' --> 1");
  t.is ((int)Lexer::commonLength ("abcde", "abcde"), 5, "commonLength 'abcde' : 'abcde' --> 5");
  t.is ((int)Lexer::commonLength ("abc", ""),        0, "commonLength 'abc' : '' --> 0");
  t.is ((int)Lexer::commonLength ("abc", "def"),     0, "commonLength 'abc' : 'def' --> 0");
  t.is ((int)Lexer::commonLength ("foobar", "foo"),  3, "commonLength 'foobar' : 'foo' --> 3");
  t.is ((int)Lexer::commonLength ("foo", "foobar"),  3, "commonLength 'foo' : 'foobar' --> 3");

  // static std::string::size_type commonLength (const std::string&, std::string::size_type, const std::string&, std::string::size_type);
  t.is ((int)Lexer::commonLength ("wonder", 0, "prowonderbread", 3), 6, "'wonder'+0 : 'prowonderbread'+3 --> 6");

  // Test all Lexer types.
  #define NO {"",Lexer::Type::word}
  struct
  {
    const char* input;
    struct
    {
      const char* token;
      Lexer::Type type;
    } results[5];
  } lexerTests[] =
  {
    // Pattern
    { "/foo/",                                        { { "/foo/",                                        Lexer::Type::pattern      }, NO, NO, NO, NO }, },
    { "/a\\/b/",                                      { { "/a\\/b/",                                      Lexer::Type::pattern      }, NO, NO, NO, NO }, },
    { "/'/",                                          { { "/'/",                                          Lexer::Type::pattern      }, NO, NO, NO, NO }, },

    // Substitution
    { "/from/to/g",                                   { { "/from/to/g",                                   Lexer::Type::substitution }, NO, NO, NO, NO }, },
    { "/from/to/",                                    { { "/from/to/",                                    Lexer::Type::substitution }, NO, NO, NO, NO }, },

    // Tag
    { "+tag",                                         { { "+tag",                                         Lexer::Type::tag          }, NO, NO, NO, NO }, },
    { "-tag",                                         { { "-tag",                                         Lexer::Type::tag          }, NO, NO, NO, NO }, },
    { "+@tag",                                        { { "+@tag",                                        Lexer::Type::tag          }, NO, NO, NO, NO }, },

    // Path
    { "/long/path/to/file.txt",                       { { "/long/path/to/file.txt",                       Lexer::Type::path         }, NO, NO, NO, NO }, },

    // Word
    { "1.foo.bar",                                    { { "1.foo.bar",                                    Lexer::Type::word         }, NO, NO, NO, NO }, },

    // Identifier
    { "foo",                                          { { "foo",                                          Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "Çirçös",                                       { { "Çirçös",                                       Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "☺",                                            { { "☺",                                            Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "name",                                         { { "name",                                         Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "f1",                                           { { "f1",                                           Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "foo.bar",                                      { { "foo.bar",                                      Lexer::Type::identifier   }, NO, NO, NO, NO }, },
    { "a1a1a1a1_a1a1_a1a1_a1a1_a1a1a1a1a1a1",         { { "a1a1a1a1_a1a1_a1a1_a1a1_a1a1a1a1a1a1",         Lexer::Type::identifier   }, NO, NO, NO, NO }, },

    // Word that starts wih 'or', which is an operator, but should be ignored.
    { "ordinary",                                     { { "ordinary",                                     Lexer::Type::identifier   }, NO, NO, NO, NO }, },

    // DOM
    { "due",                                          { { "due",                                          Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.tags",                                     { { "123.tags",                                     Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.tags.PENDING",                             { { "123.tags.PENDING",                             Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.description",                              { { "123.description",                              Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.annotations.1.description",                { { "123.annotations.1.description",                Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.annotations.1.entry",                      { { "123.annotations.1.entry",                      Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "123.annotations.1.entry.year",                 { { "123.annotations.1.entry.year",                 Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "a360fc44-315c-4366-b70c-ea7e7520b749.due",     { { "a360fc44-315c-4366-b70c-ea7e7520b749.due",     Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "12345678-1234-1234-1234-123456789012.due",     { { "12345678-1234-1234-1234-123456789012.due",     Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "system.os",                                    { { "system.os",                                    Lexer::Type::dom          }, NO, NO, NO, NO }, },
    { "rc.foo",                                       { { "rc.foo",                                       Lexer::Type::dom          }, NO, NO, NO, NO }, },

    // URL
    { "http://tasktools.org",                         { { "http://tasktools.org",                         Lexer::Type::url          }, NO, NO, NO, NO }, },
    { "https://bug.tasktools.org",                    { { "https://bug.tasktools.org",                    Lexer::Type::url          }, NO, NO, NO, NO }, },

    // String
    { "'one two'",                                    { { "'one two'",                                    Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "\"three\"",                                    { { "\"three\"",                                    Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "'\\''",                                        { { "'''",                                          Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "\"\\\"\"",                                     { { "\"\"\"",                                       Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "\"\tfoo\t\"",                                  { { "\"\tfoo\t\"",                                  Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "\"\\u20A43\"",                                 { { "\"₤3\"",                                       Lexer::Type::string       }, NO, NO, NO, NO }, },
    { "\"U+20AC4\"",                                  { { "\"€4\"",                                       Lexer::Type::string       }, NO, NO, NO, NO }, },

    // Number
    { "1",                                            { { "1",                                            Lexer::Type::number       }, NO, NO, NO, NO }, },
    { "3.14",                                         { { "3.14",                                         Lexer::Type::number       }, NO, NO, NO, NO }, },
    { "6.02217e23",                                   { { "6.02217e23",                                   Lexer::Type::number       }, NO, NO, NO, NO }, },
    { "1.2e-3.4",                                     { { "1.2e-3.4",                                     Lexer::Type::number       }, NO, NO, NO, NO }, },
    { "0x2f",                                         { { "0x2f",                                         Lexer::Type::hex          }, NO, NO, NO, NO }, },

    // Set (1,2,4-7,9)
    { "1,2",                                          { { "1,2",                                          Lexer::Type::set          }, NO, NO, NO, NO }, },
    { "1-2",                                          { { "1-2",                                          Lexer::Type::set          }, NO, NO, NO, NO }, },
    { "1-2,4",                                        { { "1-2,4",                                        Lexer::Type::set          }, NO, NO, NO, NO }, },
    { "1-2,4,6-8",                                    { { "1-2,4,6-8",                                    Lexer::Type::set          }, NO, NO, NO, NO }, },
    { "1-2,4,6-8,10-12",                              { { "1-2,4,6-8,10-12",                              Lexer::Type::set          }, NO, NO, NO, NO }, },

    // Pair
    { "name:value",                                   { { "name:value",                                   Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name=value",                                   { { "name=value",                                   Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name:=value",                                  { { "name:=value",                                  Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name.mod:value",                               { { "name.mod:value",                               Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name.mod=value",                               { { "name.mod=value",                               Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name:",                                        { { "name:",                                        Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name=",                                        { { "name=",                                        Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name.mod:",                                    { { "name.mod:",                                    Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name.mod=",                                    { { "name.mod=",                                    Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "pro:'P 1'",                                    { { "pro:'P 1'",                                    Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "rc:x",                                         { { "rc:x",                                         Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "rc.name:value",                                { { "rc.name:value",                                Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "rc.name=value",                                { { "rc.name=value",                                Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "rc.name:=value",                               { { "rc.name:=value",                               Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "due:='eow - 2d'",                              { { "due:='eow - 2d'",                              Lexer::Type::pair         }, NO, NO, NO, NO }, },
    { "name:'foo\nbar'",                              { { "name:'foo\nbar'",                              Lexer::Type::pair         }, NO, NO, NO, NO }, },

    // Operator - complete set
    { "^",                                            { { "^",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "!",                                            { { "!",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "_neg_",                                        { { "_neg_",                                        Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "_pos_",                                        { { "_pos_",                                        Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "_hastag_",                                     { { "_hastag_",                                     Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "_notag_",                                      { { "_notag_",                                      Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "*",                                            { { "*",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "/",                                            { { "/",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "%",                                            { { "%",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "+",                                            { { "+",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "-",                                            { { "-",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "<=",                                           { { "<=",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { ">=",                                           { { ">=",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { ">",                                            { { ">",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "<",                                            { { "<",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "=",                                            { { "=",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "==",                                           { { "==",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "!=",                                           { { "!=",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "!==",                                          { { "!==",                                          Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "~",                                            { { "~",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "!~",                                           { { "!~",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "and",                                          { { "and",                                          Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "or",                                           { { "or",                                           Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "xor",                                          { { "xor",                                          Lexer::Type::op           }, NO, NO, NO, NO }, },
    { "(",                                            { { "(",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },
    { ")",                                            { { ")",                                            Lexer::Type::op           }, NO, NO, NO, NO }, },

    // UUID
    { "ffffffff-ffff-ffff-ffff-ffffffffffff",         { { "ffffffff-ffff-ffff-ffff-ffffffffffff",         Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "00000000-0000-0000-0000-0000000",              { { "00000000-0000-0000-0000-0000000",              Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "00000000-0000-0000-0000",                      { { "00000000-0000-0000-0000",                      Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "00000000-0000-0000",                           { { "00000000-0000-0000",                           Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "00000000-0000",                                { { "00000000-0000",                                Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "00000000",                                     { { "00000000",                                     Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44-315c-4366-b70c-ea7e7520b749",         { { "a360fc44-315c-4366-b70c-ea7e7520b749",         Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44-315c-4366-b70c-ea7e752",              { { "a360fc44-315c-4366-b70c-ea7e752",              Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44-315c-4366-b70c",                      { { "a360fc44-315c-4366-b70c",                      Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44-315c-4366",                           { { "a360fc44-315c-4366",                           Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44-315c",                                { { "a360fc44-315c",                                Lexer::Type::uuid         }, NO, NO, NO, NO }, },
    { "a360fc44",                                     { { "a360fc44",                                     Lexer::Type::uuid         }, NO, NO, NO, NO }, },

    // Date
    { "2015-W01",                                     { { "2015-W01",                                     Lexer::Type::date         }, NO, NO, NO, NO }, },
    { "2015-02-17",                                   { { "2015-02-17",                                   Lexer::Type::date         }, NO, NO, NO, NO }, },
    { "2013-11-29T22:58:00Z",                         { { "2013-11-29T22:58:00Z",                         Lexer::Type::date         }, NO, NO, NO, NO }, },
    { "20131129T225800Z",                             { { "20131129T225800Z",                             Lexer::Type::date         }, NO, NO, NO, NO }, },
#ifdef PRODUCT_TASKWARRIOR
    { "9th",                                          { { "9th",                                          Lexer::Type::date         }, NO, NO, NO, NO }, },
    { "10th",                                         { { "10th",                                         Lexer::Type::date         }, NO, NO, NO, NO }, },
    { "today",                                        { { "today",                                        Lexer::Type::date         }, NO, NO, NO, NO }, },
#endif

    // Duration
    { "year",                                         { { "year",                                         Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "4weeks",                                       { { "4weeks",                                       Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "PT23H",                                        { { "PT23H",                                        Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "1second",                                      { { "1second",                                      Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "1s",                                           { { "1s",                                           Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "1minute",                                      { { "1minute",                                      Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "2hour",                                        { { "2hour",                                        Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "3 days",                                       { { "3 days",                                       Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "4w",                                           { { "4w",                                           Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "5mo",                                          { { "5mo",                                          Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "6 years",                                      { { "6 years",                                      Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "P1Y",                                          { { "P1Y",                                          Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "PT1H",                                         { { "PT1H",                                         Lexer::Type::duration     }, NO, NO, NO, NO }, },
    { "P1Y1M1DT1H1M1S",                               { { "P1Y1M1DT1H1M1S",                               Lexer::Type::duration     }, NO, NO, NO, NO }, },

    // Misc
    { "--",                                           { { "--",                                           Lexer::Type::separator    }, NO, NO, NO, NO }, },

    // Expression
    //   due:eom-2w
    //   due < eom + 1w + 1d
    //   ( /pattern/ or 8ad2e3db-914d-4832-b0e6-72fa04f6e331,3b6218f9-726a-44fc-aa63-889ff52be442 )
    { "(1+2)",                                        { { "(",                                            Lexer::Type::op           },
                                                        { "1",                                            Lexer::Type::number       },
                                                        { "+",                                            Lexer::Type::op           },
                                                        { "2",                                            Lexer::Type::number       },
                                                        { ")",                                            Lexer::Type::op           },                }, },
    { "description~pattern",                          { { "description",                                  Lexer::Type::dom          },
                                                        { "~",                                            Lexer::Type::op           },
                                                        { "pattern",                                      Lexer::Type::identifier   },         NO, NO }, },
    { "(+tag)",                                       { { "(",                                            Lexer::Type::op           },
                                                        { "+tag",                                         Lexer::Type::tag          },
                                                        { ")",                                            Lexer::Type::op           },         NO, NO }, },
    { "(name:value)",                                 { { "(",                                            Lexer::Type::op           },
                                                        { "name:value",                                   Lexer::Type::pair         },
                                                        { ")",                                            Lexer::Type::op           },         NO, NO }, },
  };
  #define NUM_TESTS (sizeof (lexerTests) / sizeof (lexerTests[0]))

  for (unsigned int i = 0; i < NUM_TESTS; i++)
  {
    // The isolated test puts the input string directly into the Lexer.
    Lexer isolated (lexerTests[i].input);

    for (int j = 0; j < 5; j++)
    {
      if (lexerTests[i].results[j].token[0])
      {
        // Isolated: "<token>"
        t.ok (isolated.token (token, type),                  "Isolated Lexer::token(...) --> true");
        t.is (token, lexerTests[i].results[j].token,         "  token --> " + token);
        t.is ((int)type, (int)lexerTests[i].results[j].type, "  type --> Lexer::Type::" + Lexer::typeToString (type));
      }
    }

    // The embedded test surrounds the input string with a space.
    Lexer embedded (std::string (" ") + lexerTests[i].input + " ");

    for (int j = 0; j < 5; j++)
    {
      if (lexerTests[i].results[j].token[0])
      {
        // Embedded: "<token>"
        t.ok (embedded.token (token, type),                  "Embedded Lexer::token(...) --> true");
        t.is (token, lexerTests[i].results[j].token,         "  token --> " + token);
        t.is ((int)type, (int)lexerTests[i].results[j].type, "  type --> Lexer::Type::" + Lexer::typeToString (type));
      }
    }
  }

  t.is (Lexer::typeName (Lexer::Type::uuid),         "uuid",         "Lexer::typeName (Lexer::Type::uuid)");
  t.is (Lexer::typeName (Lexer::Type::number),       "number",       "Lexer::typeName (Lexer::Type::number)");
  t.is (Lexer::typeName (Lexer::Type::hex),          "hex",          "Lexer::typeName (Lexer::Type::hex)");
  t.is (Lexer::typeName (Lexer::Type::string),       "string",       "Lexer::typeName (Lexer::Type::string)");
  t.is (Lexer::typeName (Lexer::Type::url),          "url",          "Lexer::typeName (Lexer::Type::url)");
  t.is (Lexer::typeName (Lexer::Type::pair),         "pair",         "Lexer::typeName (Lexer::Type::pair)");
  t.is (Lexer::typeName (Lexer::Type::set),          "set",          "Lexer::typeName (Lexer::Type::set)");
  t.is (Lexer::typeName (Lexer::Type::separator),    "separator",    "Lexer::typeName (Lexer::Type::separator)");
  t.is (Lexer::typeName (Lexer::Type::tag),          "tag",          "Lexer::typeName (Lexer::Type::tag)");
  t.is (Lexer::typeName (Lexer::Type::path),         "path",         "Lexer::typeName (Lexer::Type::path)");
  t.is (Lexer::typeName (Lexer::Type::substitution), "substitution", "Lexer::typeName (Lexer::Type::substitution)");
  t.is (Lexer::typeName (Lexer::Type::pattern),      "pattern",      "Lexer::typeName (Lexer::Type::pattern)");
  t.is (Lexer::typeName (Lexer::Type::op),           "op",           "Lexer::typeName (Lexer::Type::op)");
  t.is (Lexer::typeName (Lexer::Type::dom),          "dom",          "Lexer::typeName (Lexer::Type::dom)");
  t.is (Lexer::typeName (Lexer::Type::identifier),   "identifier",   "Lexer::typeName (Lexer::Type::identifier)");
  t.is (Lexer::typeName (Lexer::Type::word),         "word",         "Lexer::typeName (Lexer::Type::word)");
  t.is (Lexer::typeName (Lexer::Type::date),         "date",         "Lexer::typeName (Lexer::Type::date)");
  t.is (Lexer::typeName (Lexer::Type::duration),     "duration",     "Lexer::typeName (Lexer::Type::duration)");

  // std::string Lexer::lowerCase (const std::string& input)
  t.is (Lexer::lowerCase (""),            "",            "Lexer::lowerCase '' -> ''");
  t.is (Lexer::lowerCase ("pre01_:POST"), "pre01_:post", "Lexer::lowerCase 'pre01_:POST' -> 'pre01_:post'");

  // std::string Lexer::commify (const std::string& data)
  t.is (Lexer::commify (""),           "",                  "Lexer::commify '' -> ''");
  t.is (Lexer::commify ("1"),          "1",                 "Lexer::commify '1' -> '1'");
  t.is (Lexer::commify ("12"),         "12",                "Lexer::commify '12' -> '12'");
  t.is (Lexer::commify ("123"),        "123",               "Lexer::commify '123' -> '123'");
  t.is (Lexer::commify ("1234"),       "1,234",             "Lexer::commify '1234' -> '1,234'");
  t.is (Lexer::commify ("12345"),      "12,345",            "Lexer::commify '12345' -> '12,345'");
  t.is (Lexer::commify ("123456"),     "123,456",           "Lexer::commify '123456' -> '123,456'");
  t.is (Lexer::commify ("1234567"),    "1,234,567",         "Lexer::commify '1234567' -> '1,234,567'");
  t.is (Lexer::commify ("12345678"),   "12,345,678",        "Lexer::commify '12345678' -> '12,345,678'");
  t.is (Lexer::commify ("123456789"),  "123,456,789",       "Lexer::commify '123456789' -> '123,456,789'");
  t.is (Lexer::commify ("1234567890"), "1,234,567,890",     "Lexer::commify '1234567890' -> '1,234,567,890'");
  t.is (Lexer::commify ("1.0"),          "1.0",             "Lexer::commify '1.0' -> '1.0'");
  t.is (Lexer::commify ("12.0"),         "12.0",            "Lexer::commify '12.0' -> '12.0'");
  t.is (Lexer::commify ("123.0"),        "123.0",           "Lexer::commify '123.0' -> '123.0'");
  t.is (Lexer::commify ("1234.0"),       "1,234.0",         "Lexer::commify '1234.0' -> '1,234.0'");
  t.is (Lexer::commify ("12345.0"),      "12,345.0",        "Lexer::commify '12345.0' -> '12,345.0'");
  t.is (Lexer::commify ("123456.0"),     "123,456.0",       "Lexer::commify '123456.0' -> '123,456.0'");
  t.is (Lexer::commify ("1234567.0"),    "1,234,567.0",     "Lexer::commify '1234567.0' -> '1,234,567.0'");
  t.is (Lexer::commify ("12345678.0"),   "12,345,678.0",    "Lexer::commify '12345678.0' -> '12,345,678.0'");
  t.is (Lexer::commify ("123456789.0"),  "123,456,789.0",   "Lexer::commify '123456789.0' -> '123,456,789.0'");
  t.is (Lexer::commify ("1234567890.0"), "1,234,567,890.0", "Lexer::commify '1234567890.0' -> '1,234,567,890.0'");
  t.is (Lexer::commify ("pre"),         "pre",              "Lexer::commify 'pre' -> 'pre'");
  t.is (Lexer::commify ("pre1234"),     "pre1,234",         "Lexer::commify 'pre1234' -> 'pre1,234'");
  t.is (Lexer::commify ("1234post"),    "1,234post",        "Lexer::commify '1234post' -> '1,234post'");
  t.is (Lexer::commify ("pre1234post"), "pre1,234post",     "Lexer::commify 'pre1234post' -> 'pre1,234post'");

  // std::string Lexer::trimLeft (const std::string& in, const std::string& t /*= " "*/)
  t.is (Lexer::trimLeft (""),                     "",            "Lexer::trimLeft '' -> ''");
  t.is (Lexer::trimLeft ("   "),                  "",            "Lexer::trimLeft '   ' -> ''");
  t.is (Lexer::trimLeft ("",              " \t"), "",            "Lexer::trimLeft '' -> ''");
  t.is (Lexer::trimLeft ("xxx"),                  "xxx",         "Lexer::trimLeft 'xxx' -> 'xxx'");
  t.is (Lexer::trimLeft ("xxx",           " \t"), "xxx",         "Lexer::trimLeft 'xxx' -> 'xxx'");
  t.is (Lexer::trimLeft ("  \t xxx \t  "),        "\t xxx \t  ", "Lexer::trimLeft '  \\t xxx \\t  ' -> '\\t xxx \\t  '");
  t.is (Lexer::trimLeft ("  \t xxx \t  ", " \t"), "xxx \t  ",    "Lexer::trimLeft '  \\t xxx \\t  ' -> 'xxx \\t  '");

  // std::string Lexer::trimRight (const std::string& in, const std::string& t /*= " "*/)
  t.is (Lexer::trimRight (""),                     "",            "Lexer::trimRight '' -> ''");
  t.is (Lexer::trimRight ("   "),                  "",            "Lexer::trimRight '   ' -> ''");
  t.is (Lexer::trimRight ("",              " \t"), "",            "Lexer::trimRight '' -> ''");
  t.is (Lexer::trimRight ("xxx"),                  "xxx",         "Lexer::trimRight 'xxx' -> 'xxx'");
  t.is (Lexer::trimRight ("xxx",           " \t"), "xxx",         "Lexer::trimRight 'xxx' -> 'xxx'");
  t.is (Lexer::trimRight ("  \t xxx \t  "),        "  \t xxx \t", "Lexer::trimRight '  \\t xxx \\t  ' -> '  \\t xxx \\t'");
  t.is (Lexer::trimRight ("  \t xxx \t  ", " \t"), "  \t xxx",    "Lexer::trimRight '  \\t xxx \\t  ' -> '  \\t xxx'");

  // std::string Lexer::trim (const std::string& in, const std::string& t /*= " "*/)
  t.is (Lexer::trim (""),                     "",          "Lexer::trim '' -> ''");
  t.is (Lexer::trim ("   "),                  "",          "Lexer::trim '   ' -> ''");
  t.is (Lexer::trim ("",              " \t"), "",          "Lexer::trim '' -> ''");
  t.is (Lexer::trim ("xxx"),                  "xxx",       "Lexer::trim 'xxx' -> 'xxx'");
  t.is (Lexer::trim ("xxx",           " \t"), "xxx",       "Lexer::trim 'xxx' -> 'xxx'");
  t.is (Lexer::trim ("  \t xxx \t  "),        "\t xxx \t", "Lexer::trim '  \\t xxx \\t  ' -> '\\t xxx \\t'");
  t.is (Lexer::trim ("  \t xxx \t  ", " \t"), "xxx",       "Lexer::trim '  \\t xxx \\t  ' -> 'xxx'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
