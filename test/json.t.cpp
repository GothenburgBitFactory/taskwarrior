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

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (14);

  try
  {
    // Basic parsing tests.
    std::string input = "{}";
    std::cout << "-- j1 -------------------\n"
              << "input: " << input << "\n";
    JSON j1 (input);
    j1.tree ()->dump ();

    input = "{\"name\":123}";
    std::cout << "-- j2 -------------------\n"
              << "input: " << input << "\n";
    JSON j2 (input);
    j2.tree ()->dump ();

    input = "{\"name\":123, \"array\":[1,2,3.4], \"map\":{\"m1\":\"v1\", \"m2\":\"v2\"}}";
    std::cout << "-- j3 -------------------\n"
              << "input: " << input << "\n";
    JSON j3 (input);
    j3.tree ()->dump ();

    // Sample ticket as a parsing test.
    input = "{\n"
            "\"ticket\": { \"type\":\"add\", \"client\":\"taskwarrior 2.x\"},\n"
            "\"auth\":   { \"user\":\"paul\", \"org\":\"gbf\", \"key\":\".........\",\n"
            "            \"locale\":\"en-US\" },\n"
            "\n"
            "\"add\":    { \"description\":\"Wash the dog\",\n"
            "            \"project\":\"home\",\n"
            "            \"due\":\"20101101T000000Z\" }\n"
            "}";
    std::cout << "-- j4 -------------------\n"
              << "input: " << input << "\n";
    JSON j4 (input);
    j4.tree ()->dump ();
    std::cout << "-------------------------\n";

    // Regular unit tests.
    t.is (JSON::encode ("1\b2"), "1\\b2",  "JSON::encode \\b -> \\\\b");
    t.is (JSON::decode ("1\\b2"), "1\b2",  "JSON::decode \\\\b -> \\b");

    t.is (JSON::encode ("1\n2"), "1\\n2",  "JSON::encode \\n -> \\\\n");
    t.is (JSON::decode ("1\\n2"), "1\n2",  "JSON::decode \\\\n -> \\n");

    t.is (JSON::encode ("1\r2"), "1\\r2",  "JSON::encode \\r -> \\\\r");
    t.is (JSON::decode ("1\\r2"), "1\r2",  "JSON::decode \\\\r -> \\r");

    t.is (JSON::encode ("1\t2"), "1\\t2",  "JSON::encode \\t -> \\\\t");
    t.is (JSON::decode ("1\\t2"), "1\t2",  "JSON::decode \\\\t -> \\t");

    t.is (JSON::encode ("1\\2"), "1\\\\2", "JSON::encode \\ -> \\\\");
    t.is (JSON::decode ("1\\\\2"), "1\\2", "JSON::decode \\\\ -> \\");

    t.is (JSON::encode ("1\x2"), "1\x2",   "JSON::encode \\x -> \\x (NOP)");
    t.is (JSON::decode ("1\x2"), "1\x2",   "JSON::decode \\x -> \\x (NOP)");

    t.is (JSON::encode ("1€2"), "1€2",     "JSON::encode € -> €");
    t.is (JSON::decode ("1\\u20ac2"),      "1€2", "JSON::decode \\u20ac -> €");

/*
  {
    "ticket":
    {
      "type":"synch",
      "client":"taskd-test-suite 1.0"
    },

    "synch":
    {
      "user":
      {
        "data":
        [
          {
            "uuid":"11111111-1111-1111-1111-111111111111",
            "status":"pending",
            "description":"This is a test",
            "entry":"20110111T124000Z"
          }
        ],
        "synch":"key"
      }
    },
  
    "auth":
    {
      "org":"gbf",
      "user":"Paul Beckingham",
      "key":"K",
      "locale":"en-US"
    }
  }
*/
    input = "{\"ticket\":{\"type\":\"synch\",\"client\":\"taskd-test-suite 1.0\"},\"synch\":{\"user\":{\"data\":[{\"uuid\":\"11111111-1111-1111-1111-111111111111\",\"status\":\"pending\",\"description\":\"This is a test\",\"entry\":\"20110111T124000Z\"}],\"synch\":\"key\"}},\"auth\":{\"org\":\"gbf\",\"user\":\"Paul Beckingham\",\"key\":\"K\",\"locale\":\"en-US\"}}";
    std::cout << "-- j4 -------------------\n"
              << "input: " << input << "\n";
    JSON j5 (input);
    j5.tree ()->dump ();
  }

  catch (std::string& e) {t.diag (e);}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
