////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham.
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

#include <iostream>
#include <JSON.h>
#include <test.h>
#include <Context.h>

Context context;

const char *positive_tests[] =
{
  "{}",

  "    {    }    ",

  "[]",

  "{\"one\":1}",

  "{\n\"one\"\n:\n1\n}\n",

  "{\"name\":123, \"array\":[1,2,3.4], \"object\":{\"m1\":\"v1\", \"m2\":\"v2\"}}",

  "{\"name\":\"value\",\"array\":[\"one\",\"two\"],\"object\":{\"name2\":123,\"literal\":false}}",

  "{\n"
    "\"ticket\": { \"type\":\"add\", \"client\":\"taskwarrior 2.x\"},\n"
    "\"auth\":   { \"user\":\"paul\", \"org\":\"gbf\", \"key\":\".........\",\n"
    "            \"locale\":\"en-US\" },\n"
    "\n"
    "\"add\":    { \"description\":\"Wash the dog\",\n"
    "            \"project\":\"home\",\n"
    "            \"due\":\"20101101T000000Z\" }\n"
  "}",

  "{"
    "\"ticket\":{"
      "\"type\":\"synch\","
      "\"client\":\"taskd-test-suite 1.0\""
    "},"
    "\"synch\":{"
      "\"user\":{"
        "\"data\":["
          "{"
            "\"uuid\":\"11111111-1111-1111-1111-111111111111\","
            "\"status\":\"pending\","
            "\"description\":\"This is a test\","
            "\"entry\":\"20110111T124000Z\""
          "}"
        "],"
        "\"synch\":\"key\""
      "}"
    "},"
    "\"auth\":{"
      "\"org\":\"gbf\","
      "\"user\":\"Paul Beckingham\","
      "\"key\":\"K\","
      "\"locale\":\"en-US\""
    "}"
  "}"
};

#define NUM_POSITIVE_TESTS (sizeof (positive_tests) / sizeof (positive_tests[0]))

const char *negative_tests[] =
{
  "",
  "{",
  "}",
  "[",
  "]",
  "foo",
  "[?]"
};

#define NUM_NEGATIVE_TESTS (sizeof (negative_tests) / sizeof (negative_tests[0]))

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (NUM_POSITIVE_TESTS + NUM_NEGATIVE_TESTS + 14);

  // Positive tests.
  for (int i = 0; i < NUM_POSITIVE_TESTS; ++i)
  {
    try
    {
      json::value* root = json::parse (positive_tests[i]);
      t.ok (root, std::string ("positive: ") + positive_tests[i]);
      if (root)
      {
        t.diag (root->dump ());
        delete root;
      }
    }

    catch (const std::string& e) { t.diag (e); }
    catch (...)                  { t.diag ("Unknown error"); }
  }

  // Negative tests.
  for (int i = 0; i < NUM_NEGATIVE_TESTS; ++i)
  {
    try
    {
      json::value* root = json::parse (negative_tests[i]);
      t.is (root, NULL, std::string ("negative: ") + negative_tests[i]);
    }

    catch (const std::string& e) { t.pass (e); }
    catch (...)                  { t.fail ("Unknown error"); }
  }

  // Other tests.
  try
  {
    // Regular unit tests.
    t.is (json::encode ("1\b2"), "1\\b2",  "json::encode \\b -> \\\\b");
    t.is (json::decode ("1\\b2"), "1\b2",  "json::decode \\\\b -> \\b");

    t.is (json::encode ("1\n2"), "1\\n2",  "json::encode \\n -> \\\\n");
    t.is (json::decode ("1\\n2"), "1\n2",  "json::decode \\\\n -> \\n");

    t.is (json::encode ("1\r2"), "1\\r2",  "json::encode \\r -> \\\\r");
    t.is (json::decode ("1\\r2"), "1\r2",  "json::decode \\\\r -> \\r");

    t.is (json::encode ("1\t2"), "1\\t2",  "json::encode \\t -> \\\\t");
    t.is (json::decode ("1\\t2"), "1\t2",  "json::decode \\\\t -> \\t");

    t.is (json::encode ("1\\2"), "1\\\\2", "json::encode \\ -> \\\\");
    t.is (json::decode ("1\\\\2"), "1\\2", "json::decode \\\\ -> \\");

    t.is (json::encode ("1\x2"), "1\x2",   "json::encode \\x -> \\x (NOP)");
    t.is (json::decode ("1\x2"), "1\x2",   "json::decode \\x -> \\x (NOP)");

    t.is (json::encode ("1€2"), "1€2",     "json::encode € -> €");
    t.is (json::decode ("1\\u20ac2"),      "1€2", "json::decode \\u20ac -> €");
  }

  catch (std::string& e) {t.diag (e);}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
