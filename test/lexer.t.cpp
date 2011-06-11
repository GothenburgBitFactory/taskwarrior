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

#include <Lexer.h>
#include <Context.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (80);

  std::string input = "This is a test.";
  std::vector <std::string> tokens;
  {
    Lexer l (input);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 8, "'This is a test.' -> 'This| |is| |a| |test|.'");
  if (tokens.size () == 8)
  {
    t.is (tokens[0], "This", "'This is a test.' [0] -> 'This'");
    t.is (tokens[1], " ",    "'This is a test.' [1] -> ' '");
    t.is (tokens[2], "is",   "'This is a test.' [2] -> 'is'");
    t.is (tokens[3], " ",    "'This is a test.' [3] -> ' '");
    t.is (tokens[4], "a",    "'This is a test.' [4] -> 'a'");
    t.is (tokens[5], " ",    "'This is a test.' [5] -> ' '");
    t.is (tokens[6], "test", "'This is a test.' [6] -> 'test'");
    t.is (tokens[7], ".",    "'This is a test.' [7] -> '.'");
  }
  else
  {
    t.skip ("'This is a test.' [0] -> 'This'");
    t.skip ("'This is a test.' [1] -> ' '");
    t.skip ("'This is a test.' [2] -> 'is'");
    t.skip ("'This is a test.' [3] -> ' '");
    t.skip ("'This is a test.' [4] -> 'a'");
    t.skip ("'This is a test.' [5] -> ' '");
    t.skip ("'This is a test.' [6] -> 'test'");
    t.skip ("'This is a test.' [7] -> '.'");
  }

  input = "a12bcd345efgh6789";
  {
    Lexer l (input);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 6, "'a12bcd345efgh6789' -> 'a|12|bcd|345|efgh|6789'");
  if (tokens.size () == 6)
  {
    t.is (tokens[0], "a",    "'a12bcd345efgh6789' [0] -> 'a'");
    t.is (tokens[1], "12",   "'a12bcd345efgh6789' [1] -> '12'");
    t.is (tokens[2], "bcd",  "'a12bcd345efgh6789' [2] -> 'bcd'");
    t.is (tokens[3], "345",  "'a12bcd345efgh6789' [3] -> '345'");
    t.is (tokens[4], "efgh", "'a12bcd345efgh6789' [4] -> 'efgh'");
    t.is (tokens[5], "6789", "'a12bcd345efgh6789' [5] -> '6789'");
  }
  else
  {
    t.skip ("'a12bcd345efgh6789' [0] -> 'a'");
    t.skip ("'a12bcd345efgh6789' [1] -> '12'");
    t.skip ("'a12bcd345efgh6789' [2] -> 'bcd'");
    t.skip ("'a12bcd345efgh6789' [3] -> '345'");
    t.skip ("'a12bcd345efgh6789' [4] -> 'efgh'");
    t.skip ("'a12bcd345efgh6789' [5] -> '6789'");
  }

  // Let's throw some ugly Perl at it.
  input = "my $variable_name = 'single string';";
  {
    Lexer l (input);
    l.categorizeAsAlpha ('_');
    l.coalesceQuoted (true);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 9, "'my $variable_name = 'single string';' -> 'my| |$|variable_name| |=| |'|single string|'|;'");
  if (tokens.size () == 9)
  {
    t.is (tokens[0], "my",              "'my $variable_name = 'single string';' [0] -> 'my'");
    t.is (tokens[1], " ",               "'my $variable_name = 'single string';' [1] -> ' '");
    t.is (tokens[2], "$",               "'my $variable_name = 'single string';' [2] -> '$'");
    t.is (tokens[3], "variable_name",   "'my $variable_name = 'single string';' [3] -> 'variable_name'");
    t.is (tokens[4], " ",               "'my $variable_name = 'single string';' [4] -> ' '");
    t.is (tokens[5], "=",               "'my $variable_name = 'single string';' [5] -> '='");
    t.is (tokens[6], " ",               "'my $variable_name = 'single string';' [6] -> ' '");
    t.is (tokens[7], "'single string'", "'my $variable_name = 'single string';' [8] -> ''single string''");
    t.is (tokens[8], ";",               "'my $variable_name = 'single string';' [10] -> ';'");
  }
  else
  {
    t.skip ("'my $variable_name = 'single string';' [0] -> 'my'");
    t.skip ("'my $variable_name = 'single string';' [1] -> ' '");
    t.skip ("'my $variable_name = 'single string';' [2] -> '$'");
    t.skip ("'my $variable_name = 'single string';' [3] -> 'variable_name'");
    t.skip ("'my $variable_name = 'single string';' [4] -> ' '");
    t.skip ("'my $variable_name = 'single string';' [5] -> '='");
    t.skip ("'my $variable_name = 'single string';' [6] -> ' '");
    t.skip ("'my $variable_name = 'single string';' [8] -> ''single string''");
    t.skip ("'my $variable_name = 'single string';' [10] -> ';'");
  }

  // Now exercise all the configurable coalescence.
  input = "ab  12 'a'";
  {
    Lexer l (input);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 8, "'ab  12 'a'' -> 'ab| | |12| |'|a|''");
  if (tokens.size () == 8)
  {
    t.is (tokens[0], "ab", "'ab  12 'a'' [0] -> 'ab'");
    t.is (tokens[1], " ",  "'ab  12 'a'' [1] -> ' '");
    t.is (tokens[2], " ",  "'ab  12 'a'' [2] -> ' '");
    t.is (tokens[3], "12", "'ab  12 'a'' [3] -> '12'");
    t.is (tokens[4], " ",  "'ab  12 'a'' [4] -> ' '");
    t.is (tokens[5], "'",  "'ab  12 'a'' [5] -> '''");
    t.is (tokens[6], "a",  "'ab  12 'a'' [6] -> 'a'");
    t.is (tokens[7], "'",  "'ab  12 'a'' [7] -> '''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'ab'");
    t.skip ("'ab  12 'a'' [1] -> ' '");
    t.skip ("'ab  12 'a'' [2] -> ' '");
    t.skip ("'ab  12 'a'' [3] -> '12'");
    t.skip ("'ab  12 'a'' [4] -> ' '");
    t.skip ("'ab  12 'a'' [5] -> '''");
    t.skip ("'ab  12 'a'' [6] -> 'a'");
    t.skip ("'ab  12 'a'' [7] -> '''");
  }

  {
    Lexer l (input);
    l.coalesceAlpha (false);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 9, "'ab  12 'a'' -> 'a|b| | |12| |'|a|''");
  if (tokens.size () == 9)
  {
    t.is (tokens[0], "a",  "'ab  12 'a'' [0] -> 'a'");
    t.is (tokens[1], "b",  "'ab  12 'a'' [1] -> 'b'");
    t.is (tokens[2], " ",  "'ab  12 'a'' [2] -> ' '");
    t.is (tokens[3], " ",  "'ab  12 'a'' [3] -> ' '");
    t.is (tokens[4], "12", "'ab  12 'a'' [4] -> '12'");
    t.is (tokens[5], " ",  "'ab  12 'a'' [5] -> ' '");
    t.is (tokens[6], "'",  "'ab  12 'a'' [6] -> '''");
    t.is (tokens[7], "a",  "'ab  12 'a'' [7] -> 'a'");
    t.is (tokens[8], "'",  "'ab  12 'a'' [8] -> '''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'a'");
    t.skip ("'ab  12 'a'' [1] -> 'b'");
    t.skip ("'ab  12 'a'' [2] -> ' '");
    t.skip ("'ab  12 'a'' [3] -> ' '");
    t.skip ("'ab  12 'a'' [4] -> '12'");
    t.skip ("'ab  12 'a'' [5] -> ' '");
    t.skip ("'ab  12 'a'' [6] -> '''");
    t.skip ("'ab  12 'a'' [7] -> 'a'");
    t.skip ("'ab  12 'a'' [8] -> '''");
  }

  {
    Lexer l (input);
    l.coalesceDigits (false);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 9, "'ab  12 'a'' -> 'ab| | |1|2| |'|a|''");
  if (tokens.size () == 9)
  {
    t.is (tokens[0], "ab", "'ab  12 'a'' [0] -> 'ab'");
    t.is (tokens[1], " ",  "'ab  12 'a'' [1] -> ' '");
    t.is (tokens[2], " ",  "'ab  12 'a'' [2] -> ' '");
    t.is (tokens[3], "1",  "'ab  12 'a'' [3] -> '1'");
    t.is (tokens[4], "2",  "'ab  12 'a'' [4] -> '2'");
    t.is (tokens[5], " ",  "'ab  12 'a'' [5] -> ' '");
    t.is (tokens[6], "'",  "'ab  12 'a'' [6] -> '''");
    t.is (tokens[7], "a",  "'ab  12 'a'' [7] -> 'a'");
    t.is (tokens[8], "'",  "'ab  12 'a'' [8] -> '''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'ab'");
    t.skip ("'ab  12 'a'' [1] -> ' '");
    t.skip ("'ab  12 'a'' [2] -> ' '");
    t.skip ("'ab  12 'a'' [3] -> '1'");
    t.skip ("'ab  12 'a'' [4] -> '2'");
    t.skip ("'ab  12 'a'' [5] -> ' '");
    t.skip ("'ab  12 'a'' [6] -> '''");
    t.skip ("'ab  12 'a'' [7] -> 'a'");
    t.skip ("'ab  12 'a'' [8] -> '''");
  }

  {
    Lexer l (input);
    l.coalesceQuoted (true);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 6, "'ab  12 'a'' -> 'ab| | |12| |'a''");
  if (tokens.size () == 6)
  {
    t.is (tokens[0], "ab",  "'ab  12 'a'' [0] -> 'ab'");
    t.is (tokens[1], " ",   "'ab  12 'a'' [1] -> ' '");
    t.is (tokens[2], " ",   "'ab  12 'a'' [2] -> ' '");
    t.is (tokens[3], "12",  "'ab  12 'a'' [3] -> '12'");
    t.is (tokens[4], " ",   "'ab  12 'a'' [4] -> ' '");
    t.is (tokens[5], "'a'", "'ab  12 'a'' [5] -> ''a''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'ab'");
    t.skip ("'ab  12 'a'' [1] -> ' '");
    t.skip ("'ab  12 'a'' [2] -> ' '");
    t.skip ("'ab  12 'a'' [3] -> '12'");
    t.skip ("'ab  12 'a'' [4] -> ' '");
    t.skip ("'ab  12 'a'' [5] -> ''a''");
  }

  {
    Lexer l (input);
    l.coalesceWhite (true);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 7, "'ab  12 'a'' -> 'ab|  |12| |'|a|''");
  if (tokens.size () == 7)
  {
    t.is (tokens[0], "ab", "'ab  12 'a'' [0] -> 'ab'");
    t.is (tokens[1], "  ", "'ab  12 'a'' [1] -> '  '");
    t.is (tokens[2], "12", "'ab  12 'a'' [2] -> '12'");
    t.is (tokens[3], " ",  "'ab  12 'a'' [3] -> ' '");
    t.is (tokens[4], "'",  "'ab  12 'a'' [4] -> '''");
    t.is (tokens[5], "a",  "'ab  12 'a'' [5] -> 'a'");
    t.is (tokens[6], "'",  "'ab  12 'a'' [6] -> '''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'ab'");
    t.skip ("'ab  12 'a'' [1] -> '  '");
    t.skip ("'ab  12 'a'' [2] -> '12'");
    t.skip ("'ab  12 'a'' [3] -> ' '");
    t.skip ("'ab  12 'a'' [4] -> '''");
    t.skip ("'ab  12 'a'' [5] -> 'a'");
    t.skip ("'ab  12 'a'' [6] -> '''");
  }

  {
    Lexer l (input);
    l.skipWhitespace (true);
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 5, "'ab  12 'a'' -> 'ab|12|'|a|''");
  if (tokens.size () == 5)
  {
    t.is (tokens[0], "ab", "'ab  12 'a'' [0] -> 'ab'");
    t.is (tokens[1], "12", "'ab  12 'a'' [1] -> '12'");
    t.is (tokens[2], "'",  "'ab  12 'a'' [2] -> '''");
    t.is (tokens[3], "a",  "'ab  12 'a'' [3] -> 'a'");
    t.is (tokens[4], "'",  "'ab  12 'a'' [4] -> '''");
  }
  else
  {
    t.skip ("'ab  12 'a'' [0] -> 'ab'");
    t.skip ("'ab  12 'a'' [1] -> '12'");
    t.skip ("'ab  12 'a'' [2] -> '''");
    t.skip ("'ab  12 'a'' [3] -> 'a'");
    t.skip ("'ab  12 'a'' [4] -> '''");
  }

  // Special tokens
  input = "a := 1";
  {
    Lexer l (input);
    l.skipWhitespace (true);
    l.specialToken (":=");
    l.tokenize (tokens);
  }

  t.is (tokens.size (), (size_t) 3, "'a := 1' -> 'a|:=|1'");
  if (tokens.size () == 3)
  {
    t.is (tokens[0], "a",  "'a := 1' [0] -> 'a'");
    t.is (tokens[1], ":=", "'a := 1' [1] -> ':='");
    t.is (tokens[2], "1",  "'a := 1' [2] -> '1'");
  }
  else
  {
    t.skip ("'a := 1' [0] -> 'a'");
    t.skip ("'a := 1' [1] -> ':='");
    t.skip ("'a := 1' [2] -> '1'");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

