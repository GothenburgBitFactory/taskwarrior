////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Göteborg Bit Factory.
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
#include <test.h>
#include <Lexer.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (212);

  std::vector <std::pair <std::string, Lexer::Type> > tokens;
  std::string token;
  Lexer::Type type;

  // White space detection.
  t.notok (Lexer::is_ws (0x0041), "U+0041 (A) is not ws");
  t.ok (Lexer::is_ws (0x0020), "U+0020 is_ws");
  t.ok (Lexer::is_ws (0x0009), "U+0009 is_ws");
  t.ok (Lexer::is_ws (0x000A), "U+000A is_ws");
  t.ok (Lexer::is_ws (0x000B), "U+000B is_ws");
  t.ok (Lexer::is_ws (0x000C), "U+000C is_ws");
  t.ok (Lexer::is_ws (0x000D), "U+000D is_ws");
  t.ok (Lexer::is_ws (0x0085), "U+0085 is_ws");
  t.ok (Lexer::is_ws (0x00A0), "U+00A0 is_ws");
  t.ok (Lexer::is_ws (0x1680), "U+1680 is_ws"); // 10
  t.ok (Lexer::is_ws (0x180E), "U+180E is_ws");
  t.ok (Lexer::is_ws (0x2000), "U+2000 is_ws");
  t.ok (Lexer::is_ws (0x2001), "U+2001 is_ws");
  t.ok (Lexer::is_ws (0x2002), "U+2002 is_ws");
  t.ok (Lexer::is_ws (0x2003), "U+2003 is_ws");
  t.ok (Lexer::is_ws (0x2004), "U+2004 is_ws");
  t.ok (Lexer::is_ws (0x2005), "U+2005 is_ws");
  t.ok (Lexer::is_ws (0x2006), "U+2006 is_ws");
  t.ok (Lexer::is_ws (0x2007), "U+2007 is_ws");
  t.ok (Lexer::is_ws (0x2008), "U+2008 is_ws"); // 20
  t.ok (Lexer::is_ws (0x2009), "U+2009 is_ws");
  t.ok (Lexer::is_ws (0x200A), "U+200A is_ws");
  t.ok (Lexer::is_ws (0x2028), "U+2028 is_ws");
  t.ok (Lexer::is_ws (0x2029), "U+2029 is_ws");
  t.ok (Lexer::is_ws (0x202F), "U+202F is_ws");
  t.ok (Lexer::is_ws (0x205F), "U+205F is_ws");
  t.ok (Lexer::is_ws (0x3000), "U+3000 is_ws");

  // static bool Lexer::boundary(int, int);
  t.ok    (Lexer::boundary (' ', 'a'), "' ' --> 'a' = boundary");
  t.ok    (Lexer::boundary ('a', ' '), "'a' --> ' ' = boundary");
  t.ok    (Lexer::boundary (' ', '+'), "' ' --> '+' = boundary");
  t.ok    (Lexer::boundary (' ', ','), "' ' --> ',' = boundary");
  t.notok (Lexer::boundary ('3', '4'), "'3' --> '4' = boundary");
  t.ok    (Lexer::boundary ('(', '('), "'(' --> '(' = boundary");
  t.notok (Lexer::boundary ('r', 'd'), "'r' --> 'd' = boundary");

  // Should result in no tokens.
  Lexer l0 ("");
  t.notok (l0.token (token, type), "'' --> no tokens");

  // Should result in no tokens.
  Lexer l1 ("       \t ");
  t.notok (l1.token (token, type), "'       \\t ' --> no tokens");

  // \u20ac = Euro symbol.
  Lexer l2 (" one 'two \\'three\\''+456-(1.3*2 - 0x12) \\u0041 1.2e-3.4    foo.bar and '\\u20ac'");

  tokens.clear ();
  while (l2.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is (tokens[0].first,                      "one",        "tokens[0] = 'left'"); // 30
  t.is (Lexer::type_name (tokens[0].second),  "Identifier", "tokens[0] = Identifier");

  t.is (tokens[1].first,                      "two 'three'", "tokens[1] = 'two \\'three\\''");
  t.is (Lexer::type_name (tokens[1].second),  "String",     "tokens[1] = String");

  t.is (tokens[2].first,                      "+",          "tokens[2] = '+'");
  t.is (Lexer::type_name (tokens[2].second),  "Operator",   "tokens[2] = Operator");

  t.is (tokens[3].first,                      "456",        "tokens[3] = '456'");
  t.is (Lexer::type_name (tokens[3].second),  "Number",     "tokens[3] = Number");

  t.is (tokens[4].first,                      "-",          "tokens[4] = '-'");
  t.is (Lexer::type_name (tokens[4].second),  "Operator",   "tokens[4] = Operator");

  t.is (tokens[5].first,                      "(",          "tokens[5] = '('"); // 40
  t.is (Lexer::type_name (tokens[5].second),  "Operator",   "tokens[5] = Operator");

  t.is (tokens[6].first,                      "1.3",        "tokens[6] = '1.3'");
  t.is (Lexer::type_name (tokens[6].second),  "Decimal",    "tokens[6] = Decimal");

  t.is (tokens[7].first,                      "*",          "tokens[7] = '*'");
  t.is (Lexer::type_name (tokens[7].second),  "Operator",   "tokens[7] = Operator");

  t.is (tokens[8].first,                      "2",          "tokens[8] = '2'");
  t.is (Lexer::type_name (tokens[8].second),  "Number",     "tokens[8] = Number");

  t.is (tokens[9].first,                      "-",          "tokens[9] = '-'");
  t.is (Lexer::type_name (tokens[9].second),  "Operator",   "tokens[9] = Operator");

  t.is (tokens[10].first,                     "0x12",       "tokens[10] = '0x12'"); // 50
  t.is (Lexer::type_name (tokens[10].second), "Hex",        "tokens[10] = Hex");

  t.is (tokens[11].first,                     ")",          "tokens[11] = ')'");
  t.is (Lexer::type_name (tokens[11].second), "Operator",   "tokens[11] = Operator");

  t.is (tokens[12].first,                     "A",          "tokens[12] = \\u0041 --> 'A'");
  t.is (Lexer::type_name (tokens[12].second), "Identifier", "tokens[12] = Identifier");

  t.is (tokens[13].first,                     "1.2e-3.4",   "tokens[13] = '1.2e-3.4'");
  t.is (Lexer::type_name (tokens[13].second), "Decimal",    "tokens[13] = Decimal");

  t.is (tokens[14].first,                     "foo.bar",    "tokens[14] = 'foo.bar'");
  t.is (Lexer::type_name (tokens[14].second), "Identifier", "tokens[14] = Identifier");

  t.is (tokens[15].first,                     "and",        "tokens[15] = 'and'"); // 60
  t.is (Lexer::type_name (tokens[15].second), "Operator",   "tokens[15] = Operator");

  t.is (tokens[16].first,                     "€",          "tokens[16] = \\u20ac --> '€'");
  t.is (Lexer::type_name (tokens[16].second), "String",     "tokens[16] = String");

  // Test for ISO-8601 dates (favoring dates in ambiguous cases).
  Lexer l3 ("1 12 123 1234 12345 123456 1234567 12345678 20131129T225800Z 2013-11-29T22:58:00Z");
  l3.ambiguity (true);
  tokens.clear ();
  while (l3.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  10,                     "10 tokens");
  t.is (tokens[0].first,                      "1",                    "tokens[0] == '1'");
  t.is (tokens[0].second,                     Lexer::typeNumber,      "tokens[0] == typeNumber");
  t.is (tokens[1].first,                      "12",                   "tokens[1] == '12'");
  t.is (tokens[1].second,                     Lexer::typeDate,        "tokens[1] == typeDate");
  t.is (tokens[2].first,                      "123",                  "tokens[2] == '123'");
  t.is (tokens[2].second,                     Lexer::typeNumber,      "tokens[2] == typeNumber"); // 70
  t.is (tokens[3].first,                      "1234",                 "tokens[3] == '1234'");
  t.is (tokens[3].second,                     Lexer::typeDate,        "tokens[3] == typeDate");
  t.is (tokens[4].first,                      "12345",                "tokens[4] == '12345'");
  t.is (tokens[4].second,                     Lexer::typeNumber,      "tokens[4] == typeNumber");
  t.is (tokens[5].first,                      "123456",               "tokens[5] == '123456'");
  t.is (tokens[5].second,                     Lexer::typeDate,        "tokens[5] == typeDate");
  t.is (tokens[6].first,                      "1234567",              "tokens[6] == '1234567'");
  t.is (tokens[6].second,                     Lexer::typeNumber,      "tokens[6] == typeNumber");
  t.is (tokens[7].first,                      "12345678",             "tokens[7] == '12345678'");
  t.is (tokens[7].second,                     Lexer::typeNumber,      "tokens[7] == typeNumber"); // 80
  t.is (tokens[8].first,                      "20131129T225800Z",     "tokens[8] == '20131129T225800Z'");
  t.is (tokens[8].second,                     Lexer::typeDate,        "tokens[8] == typeDate");
  t.is (tokens[9].first,                      "2013-11-29T22:58:00Z", "tokens[9] == '2013-11-29T22:58:00Z'");
  t.is (tokens[9].second,                     Lexer::typeDate,        "tokens[9] == typeDate");

  // Test for ISO-8601 dates (favoring numbers in ambiguous cases).
  Lexer l4 ("1 12 123 1234 12345 123456 1234567 12345678 20131129T225800Z 2013-11-29T22:58:00Z");
  l4.ambiguity (false);
  tokens.clear ();
  while (l4.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  10,                     "10 tokens");
  t.is (tokens[0].first,                      "1",                    "tokens[0] == '1'");
  t.is (tokens[0].second,                     Lexer::typeNumber,      "tokens[0] == typeNumber");
  t.is (tokens[1].first,                      "12",                   "tokens[1] == '12'");
  t.is (tokens[1].second,                     Lexer::typeNumber,      "tokens[1] == typeNumber");
  t.is (tokens[2].first,                      "123",                  "tokens[2] == '123'"); // 90
  t.is (tokens[2].second,                     Lexer::typeNumber,      "tokens[2] == typeNumber");
  t.is (tokens[3].first,                      "1234",                 "tokens[3] == '1234'");
  t.is (tokens[3].second,                     Lexer::typeNumber,      "tokens[3] == typeNumber");
  t.is (tokens[4].first,                      "12345",                "tokens[4] == '12345'");
  t.is (tokens[4].second,                     Lexer::typeNumber,      "tokens[4] == typeNumber");
  t.is (tokens[5].first,                      "123456",               "tokens[5] == '123456'");
  t.is (tokens[5].second,                     Lexer::typeNumber,      "tokens[5] == typeNumber");
  t.is (tokens[6].first,                      "1234567",              "tokens[6] == '1234567'");
  t.is (tokens[6].second,                     Lexer::typeNumber,      "tokens[6] == typeNumber");
  t.is (tokens[7].first,                      "12345678",             "tokens[7] == '12345678'"); // 100
  t.is (tokens[7].second,                     Lexer::typeNumber,      "tokens[7] == typeNumber");
  t.is (tokens[8].first,                      "20131129T225800Z",     "tokens[8] == '20131129T225800Z'");
  t.is (tokens[8].second,                     Lexer::typeDate,        "tokens[8] == typeDate");
  t.is (tokens[9].first,                      "2013-11-29T22:58:00Z", "tokens[9] == '2013-11-29T22:58:00Z'");
  t.is (tokens[9].second,                     Lexer::typeDate,        "tokens[9] == typeDate");

  // Test for durations
  Lexer l5 ("1second 1minute 2hour 3 days 4w 5mo 6 years");
  tokens.clear ();
  while (l5.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  7,                      "7 tokens");
  t.is (tokens[0].first,                      "1second",              "tokens[0] == '1second'");
  t.is (tokens[0].second,                     Lexer::typeDuration,    "tokens[0] == typeDuration");
  t.is (tokens[1].first,                      "1minute",              "tokens[1] == '1minute'");
  t.is (tokens[1].second,                     Lexer::typeDuration,    "tokens[1] == typeDuration"); // 110
  t.is (tokens[2].first,                      "2hour",                "tokens[2] == '2hour'");
  t.is (tokens[2].second,                     Lexer::typeDuration,    "tokens[2] == typeDuration");
  t.is (tokens[3].first,                      "3 days",               "tokens[3] == '3 days'");
  t.is (tokens[3].second,                     Lexer::typeDuration,    "tokens[3] == typeDuration");
  t.is (tokens[4].first,                      "4w",                   "tokens[4] == '4w'");
  t.is (tokens[4].second,                     Lexer::typeDuration,    "tokens[4] == typeDuration");
  t.is (tokens[5].first,                      "5mo",                  "tokens[5] == '5mo'");
  t.is (tokens[5].second,                     Lexer::typeDuration,    "tokens[5] == typeDuration");
  t.is (tokens[6].first,                      "6 years",              "tokens[6] == '6 years'");
  t.is (tokens[6].second,                     Lexer::typeDuration,    "tokens[6] == typeDuration"); // 120

  // All the Eval operators.
  Lexer l6 ("P1Y PT1H P1Y1M1DT1H1M1S 1s 1second");
  tokens.clear ();
  while (l6.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  5,                      "5 ISO periods");
  t.is (tokens[0].first,                      "P1Y",                  "tokens[0] == 'P1Y'");
  t.is (tokens[0].second,                     Lexer::typeDuration,    "tokens[0] == typeDuration");
  t.is (tokens[1].first,                      "PT1H",                 "tokens[1] == 'PT1H'");
  t.is (tokens[1].second,                     Lexer::typeDuration,    "tokens[1] == typeDuration");
  t.is (tokens[2].first,                      "P1Y1M1DT1H1M1S",       "tokens[2] == 'P1Y1M1DT1H1M1S'");
  t.is (tokens[2].second,                     Lexer::typeDuration,    "tokens[2] == typeDuration");
  t.is (tokens[3].first,                      "1s",                   "tokens[3] == '1s'");
  t.is (tokens[3].second,                     Lexer::typeDuration,    "tokens[3] == typeDuration");
  t.is (tokens[4].first,                      "1second",              "tokens[4] == '1second'");
  t.is (tokens[4].second,                     Lexer::typeDuration,    "tokens[4] == typeDuration");

  // All the Eval operators.
  Lexer l7 ("and xor or <= >= !~ != == = ^ > ~ ! * / % + - < ( )");
  tokens.clear ();
  while (l7.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  21,                     "21 operators");
  t.is (tokens[0].first,                      "and",                  "tokens[0] == 'and'");
  t.is (tokens[0].second,                     Lexer::typeOperator,    "tokens[0] == typeOperator"); // 130
  t.is (tokens[1].first,                      "xor",                  "tokens[1] == 'xor'");
  t.is (tokens[1].second,                     Lexer::typeOperator,    "tokens[1] == typeOperator");
  t.is (tokens[2].first,                      "or",                   "tokens[2] == 'or'");
  t.is (tokens[2].second,                     Lexer::typeOperator,    "tokens[2] == typeOperator");
  t.is (tokens[3].first,                      "<=",                   "tokens[3] == '<='");
  t.is (tokens[3].second,                     Lexer::typeOperator,    "tokens[3] == typeOperator");
  t.is (tokens[4].first,                      ">=",                   "tokens[4] == '>='");
  t.is (tokens[4].second,                     Lexer::typeOperator,    "tokens[4] == typeOperator");
  t.is (tokens[5].first,                      "!~",                   "tokens[5] == '!~'");
  t.is (tokens[5].second,                     Lexer::typeOperator,    "tokens[5] == typeOperator"); // 140
  t.is (tokens[6].first,                      "!=",                   "tokens[6] == '!='");
  t.is (tokens[6].second,                     Lexer::typeOperator,    "tokens[6] == typeOperator");
  t.is (tokens[7].first,                      "==",                   "tokens[7] == '=='");
  t.is (tokens[7].second,                     Lexer::typeOperator,    "tokens[7] == typeOperator");
  t.is (tokens[8].first,                      "=",                    "tokens[8] == '='");
  t.is (tokens[8].second,                     Lexer::typeOperator,    "tokens[8] == typeOperator");
  t.is (tokens[9].first,                      "^",                    "tokens[9] == '^'");
  t.is (tokens[9].second,                     Lexer::typeOperator,    "tokens[9] == typeOperator");
  t.is (tokens[10].first,                     ">",                    "tokens[10] == '>'");
  t.is (tokens[10].second,                    Lexer::typeOperator,    "tokens[10] == typeOperator"); // 150
  t.is (tokens[11].first,                     "~",                    "tokens[11] == '~'");
  t.is (tokens[11].second,                    Lexer::typeOperator,    "tokens[11] == typeOperator");
  t.is (tokens[12].first,                     "!",                    "tokens[12] == '!'");
  t.is (tokens[12].second,                    Lexer::typeOperator,    "tokens[12] == typeOperator");
  t.is (tokens[13].first,                     "*",                    "tokens[13] == '*'");
  t.is (tokens[13].second,                    Lexer::typeOperator,    "tokens[13] == typeOperator");
  t.is (tokens[14].first,                     "/",                    "tokens[14] == '/'");
  t.is (tokens[14].second,                    Lexer::typeOperator,    "tokens[14] == typeOperator");
  t.is (tokens[15].first,                     "%",                    "tokens[15] == '%'");
  t.is (tokens[15].second,                    Lexer::typeOperator,    "tokens[15] == typeOperator"); // 160
  t.is (tokens[16].first,                     "+",                    "tokens[16] == '+'");
  t.is (tokens[16].second,                    Lexer::typeOperator,    "tokens[16] == typeOperator");
  t.is (tokens[17].first,                     "-",                    "tokens[17] == '-'");
  t.is (tokens[17].second,                    Lexer::typeOperator,    "tokens[17] == typeOperator");
  t.is (tokens[18].first,                     "<",                    "tokens[18] == '<'");
  t.is (tokens[18].second,                    Lexer::typeOperator,    "tokens[18] == typeOperator");
  t.is (tokens[19].first,                     "(",                    "tokens[19] == '('");
  t.is (tokens[19].second,                    Lexer::typeOperator,    "tokens[19] == typeOperator");
  t.is (tokens[20].first,                     ")",                    "tokens[20] == ')'");
  t.is (tokens[20].second,                    Lexer::typeOperator,    "tokens[20] == typeOperator"); // 170

  // Test ordinal dates.
  Lexer l8 ("9th 10th");
  l8.ambiguity (false);
  tokens.clear ();
  while (l8.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  2,                      "2 tokens");
  t.is (tokens[0].first,                      "9th",                  "tokens[0] == '9th'");
  t.is (tokens[0].second,                     Lexer::typeIdentifier,  "tokens[0] == typeIdentifier");
  t.is (tokens[1].first,                      "10th",                 "tokens[1] == '10th'");
  t.is (tokens[1].second,                     Lexer::typeIdentifier,  "tokens[1] == typeIdentifier");

  // Test tag recognition.
  Lexer l9 ("+with -WITHOUT + 2");
  l9.ambiguity (false);
  tokens.clear ();
  while (l9.token (token, type))
  {
    std::cout << "# «" << token << "» " << type  << " " << Lexer::type_name (type) << "\n";
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  t.is ((int)tokens.size (),                  4,                      "4 tokens");
  t.is (tokens[0].first,                      "+with",                "tokens[0] == '+with'");
  t.is (tokens[0].second,                     Lexer::typeTag,         "tokens[0] == typeTag");
  t.is (tokens[1].first,                      "-WITHOUT",             "tokens[1] == '-WITHOUT'");
  t.is (tokens[1].second,                     Lexer::typeTag,         "tokens[1] == typeTag");
  t.is (tokens[2].first,                      "+",                    "tokens[2] == '+'");
  t.is (tokens[2].second,                     Lexer::typeOperator,    "tokens[2] == typeOperator");
  t.is (tokens[3].first,                      "2",                    "tokens[3] == '2'");
  t.is (tokens[3].second,                     Lexer::typeNumber,      "tokens[3] == typeNumber");

  // void word_split (std::vector<std::string>&, const std::string&);
  std::string unsplit = " ( A or B ) ";
  std::vector <std::string> items;
  Lexer::word_split (items, unsplit);
  t.is (items.size (), (size_t) 5, "word_split ' ( A or B ) '");
  t.is (items[0], "(",             "word_split ' ( A or B ) ' -> [0] '('");
  t.is (items[1], "A",             "word_split ' ( A or B ) ' -> [1] 'A'");
  t.is (items[2], "or",            "word_split ' ( A or B ) ' -> [2] 'or'");
  t.is (items[3], "B",             "word_split ' ( A or B ) ' -> [3] 'B'");
  t.is (items[4], ")",             "word_split ' ( A or B ) ' -> [4] ')'");

  // Test simple mode with contrived tokens that ordinarily split.
  unsplit = "  +-* a+b 12.3e4 'c d'";
  Lexer::word_split (items, unsplit);
  t.is (items.size (), (size_t) 4, "word_split '  +-* a+b 12.3e4 'c d''");
  t.is (items[0], "+-*",           "word_split '  +-* a+b 12.3e4 'c d'' -> [0] '+-*'");
  t.is (items[1], "a+b",           "word_split '  +-* a+b 12.3e4 'c d'' -> [1] 'a+b'");
  t.is (items[2], "12.3e4",        "word_split '  +-* a+b 12.3e4 'c d'' -> [2] '12.3e4'");
  t.is (items[3], "c d",           "word_split '  +-* a+b 12.3e4 'c d'' -> [3] 'c d'");

  // Test common expression element.
  unsplit = "name=value";
  Lexer::token_split (items, unsplit);
  t.is (items.size (), (size_t) 3, "token_split 'name=value'");
  if (items.size () == 3)
  {
    t.is (items[0], "name",          "token_split 'name=value' -> [0] 'name'");
    t.is (items[1], "=",             "token_split 'name=value' -> [1] '='");
    t.is (items[2], "value",         "token_split 'name=value' -> [2] 'value'");
  }
  else
  {
    t.fail ("token_split 'name=value' -> [0] 'name'");
    t.fail ("token_split 'name=value' -> [1] '='");
    t.fail ("token_split 'name=value' -> [2] 'value'");
  }

  // Test unterminated tokens.
  unsplit = " ordinary ";
  Lexer::token_split (items, unsplit);
  t.is (items.size (), (size_t) 1, "token_split 'ordinary' --> 1 token");
  t.is (items[0], "ordinary",      "token_split 'ordinary' --> 'ordinary'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
