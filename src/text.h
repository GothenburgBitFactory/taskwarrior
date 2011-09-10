////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_TEXT
#define INCLUDED_TEXT
#define L10N                                           // Localization complete.

#include <string>
#include <vector>
#include <cmake.h>

// text.cpp, Non-UTF-8 aware.
void wrapText (std::vector <std::string>&, const std::string&, const int, bool);
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
std::string unquoteText (const std::string&);
int longestWord (const std::string&);
int longestLine (const std::string&);
void extractLine (std::string&, std::string&, int, bool);
void splitq (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const std::string&);
void split_minimal (std::vector<std::string>&, const std::string&, const char);
void split_minimal (std::vector<std::string>&, const std::string&, const std::string&);
void join (std::string&, const std::string&, const std::vector<std::string>&);
void join (std::string&, const std::string&, const std::vector<int>&);
std::string commify (const std::string&);
std::string lowerCase (const std::string&);
std::string upperCase (const std::string&);
std::string ucFirst (const std::string&);
const std::string str_replace (std::string&, const std::string&, const std::string&);
const char* optionalBlankLine ();
void guess (const std::string&, std::vector<std::string>&, std::string&);
bool nontrivial (const std::string&);
bool digitsOnly (const std::string&);
bool noSpaces (const std::string&);
bool noVerticalSpace (const std::string&);
bool isWordStart (const std::string&, std::string::size_type);
bool isWordEnd (const std::string&, std::string::size_type);
bool isTokenEnd (const std::string&, std::string::size_type);
bool isPunctuation (char);
bool compare (const std::string&, const std::string&, bool sensitive = true);
bool closeEnough (const std::string&, const std::string&);
std::string::size_type find (const std::string&, const std::string&, bool sensitive = true);
std::string::size_type find (const std::string&, const std::string&, std::string::size_type, bool sensitive = true);
int strippedLength (const std::string&);
std::string cutOff (const std::string&, std::string::size_type);
const std::string format (char);
const std::string format (int);
const std::string formatHex (int);
const std::string format (float, int, int);
const std::string format (double, int, int);
const std::string format (double);
const std::string format (const std::string&, const std::string&);
const std::string format (const std::string&, int);
const std::string format (const std::string&, const std::string&, const std::string&);
const std::string format (const std::string&, const std::string&, int);
const std::string format (const std::string&, int, const std::string&);
const std::string format (const std::string&, int, int);
const std::string format (const std::string&, int, double);
const std::string format (const std::string&, const std::string&, const std::string&, const std::string&);

std::string leftJustify (const int, const int);
std::string leftJustify (const std::string&, const int);
std::string rightJustifyZero (const int, const int);
std::string rightJustify (const int, const int);
std::string rightJustify (const std::string&, const int);

#endif
////////////////////////////////////////////////////////////////////////////////
