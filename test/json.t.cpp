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
  UnitTest t (19);

  try
  {
    // j1
    std::string input = "{}";
    std::cout << "# -- j1 -------------------\n"
              << "# input: " << input << "\n";
    json::value* root = json::parse (input);
    t.ok (root, "j1 parse ok");
    if (root)
    {
      t.diag ("output: " + root->dump ());
      delete root;
    }
    else
      t.fail ("j1 parse error");

    // j2
    input = "{\"name\":123}";
    std::cout << "# -- j2 -------------------\n"
              << "# input: " << input << "\n";
    root = json::parse (input);
    t.ok (root, "j2 parse ok");
    if (root)
    {
      t.diag ("output: " + root->dump ());
      delete root;
    }
    else
      t.fail ("j2 parse error");

    // j3
    input = "{\"name\":123, \"array\":[1,2,3.4], \"map\":{\"m1\":\"v1\", \"m2\":\"v2\"}}";
    std::cout << "# -- j3 -------------------\n"
              << "# input: " << input << "\n";
    root = json::parse (input);
    t.ok (root, "j3 parse ok");
    if (root)
    {
      t.diag ("output: " + root->dump ());
      delete root;
    }
    else
      t.fail ("j3 parse error");

    // j4
    input = "{\n"
            "\"ticket\": { \"type\":\"add\", \"client\":\"taskwarrior 2.x\"},\n"
            "\"auth\":   { \"user\":\"paul\", \"org\":\"gbf\", \"key\":\".........\",\n"
            "            \"locale\":\"en-US\" },\n"
            "\n"
            "\"add\":    { \"description\":\"Wash the dog\",\n"
            "            \"project\":\"home\",\n"
            "            \"due\":\"20101101T000000Z\" }\n"
            "}";
    std::cout << "# -- j4 -------------------\n"
              << "# input: " << input << "\n";
    root = json::parse (input);
    t.ok (root, "j4 parse ok");
    if (root)
    {
      t.diag ("output: " + root->dump ());
      delete root;
    }
    else
      t.fail ("j4 parse error");

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
    std::cout << "# -- j5 -------------------\n"
              << "# input: " << input << "\n";
    root = json::parse (input);
    t.ok (root, "j5 parse ok");
    if (root)
    {
      t.diag ("output: " + root->dump ());
      delete root;
    }
    else
      t.fail ("j5 parse error");
  }

  catch (std::string& e) {t.diag (e);}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
