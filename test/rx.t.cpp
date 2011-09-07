////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <RX.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest ut (21);

  std::string text = "This is a test.";

  RX r1 ("i. ", true);
  ut.ok (r1.match (text), text + " =~ /i. /");

  std::vector <std::string> matches;
  ut.ok (r1.match (matches, text), text + " =~ /i. /");
  ut.ok (matches.size () == 2, "2 match");
  ut.is (matches[0], "is ", "$1 == is\\s");
  ut.is (matches[1], "is ", "$1 == is\\s");

  text = "abcdefghijklmnopqrstuvwxyz";

  RX r3 ("t..", true);
  ut.ok (r3.match (text), "t..");

  RX r4 ("T..", false);
  ut.ok (r4.match (text), "T..");

  RX r5 ("T..", true);
  ut.ok (!r5.match (text), "! T..");

  text = "this is a test of the regex engine.";
  //      |...:....|....:....|....:....|....:

  RX r6 ("^this");
  ut.ok (r6.match (text),      "^this matches");

  RX r7 ("engine\\.$");
  ut.ok (r7.match (text), "engine\\.$ matches");

  std::vector <std::string> results;
  std::vector <int> start;
  std::vector <int> end;
  RX r8 ("e..", true);
  ut.ok (r8.match (results, text), "e.. there are matches");
  ut.ok (r8.match (start, end, text), "e.. there are matches");
  ut.is (results.size (), (size_t) 4, "e.. == 4 matches");
  ut.is (results[0], "est", "e..[0] == 'est'");
  ut.is (start[0],      11, "e..[0] == 11->");
  ut.is (end[0],        14, "e..[0] == ->14");

  results.clear ();
  RX r9 ("e", true);
  ut.ok (r9.match (results, text),           "e there are matches");
  ut.is (results.size (), (size_t) 6,        "e == 6 matches");

  start.clear ();
  end.clear ();
  ut.ok (r9.match (start, end, text),        "e there are matches");
  ut.is (start.size (), (size_t) 6,          "e == 6 matches");

  RX r10 ("\\bthe\\b");
  text = "this is the end.";
  ut.ok (r10.match (text), text + " =~ /\\bthe\\b/");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

