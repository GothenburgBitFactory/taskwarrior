////////////////////////////////////////////////////////////////////////////////
// Tegelsten - building blocks for UI
//
// Copyright 2010, Paul Beckingham, Federico Hernandez.
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
#include <rx.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest ut (15);

  std::string text = "This is a test.";

  ut.ok (regexMatch (text, "i. ", true), text + " =~ /i. /");

  std::vector <std::string> matches;
  ut.ok (regexMatch (matches, text, "(i.) ", false), text + " =~ /(i.) /");
  ut.ok (matches.size () == 1, "1 match");
  ut.is (matches[0], "is", "$1 == is");

  text = "abcdefghijklmnopqrstuvwxyz";

  ut.ok (regexMatch (text, "t..", true), "t..");
  ut.ok (regexMatch (text, "T..", false), "T..");
  ut.ok (!regexMatch (text, "T..", true), "! T..");

  text = "this is a test of the regex engine.";
  //      |...:....|....:....|....:....|....:

  ut.ok (regexMatch (text, "^this"),      "^this matches");
  ut.ok (regexMatch (text, "engine\\.$"), "engine\\.$ matches");

  std::vector <std::string> results;
  std::vector <int> start;
  std::vector <int> end;
  ut.ok (regexMatch (results, text,    "(e..)", true), "(e..) there are matches");
  ut.ok (regexMatch (start, end, text, "(e..)", true), "(e..) there are matches");
  ut.is (results.size (), (size_t) 1, "(e..) == 1 match");
  ut.is (results[0], "est", "(e..)[0] == 'est'");
  ut.is (start[0],      11, "(e..)[0] == 11->");
  ut.is (end[0],        14, "(e..)[0] == ->14");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

