////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include <string>
#include <vector>
#include "../auto.h"

// text.cpp, Non-UTF-8 aware.
void wrapText (std::vector <std::string>&, const std::string&, const int);
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
std::string unquoteText (const std::string&);
void extractLine (std::string&, std::string&, int);
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
const char* optionalBlankLine ();
void guess (const std::string&, std::vector<std::string>&, std::string&);
bool digitsOnly (const std::string&);
bool noSpaces (const std::string&);
bool noVerticalSpace (const std::string&);
bool isWordStart (const std::string&, std::string::size_type);
bool isWordEnd (const std::string&, std::string::size_type);
bool isPunctuation (char);
bool compare (const std::string&, const std::string&, bool sensitive = true);
std::string::size_type find (const std::string&, const std::string&, bool sensitive = true);
std::string::size_type find (const std::string&, const std::string&, std::string::size_type, bool sensitive = true);
int strippedLength (const std::string&);
std::string cutOff (const std::string&, std::string::size_type);
std::string format (char);
std::string format (int);
std::string formatHex (int);
std::string format (float, int, int);
std::string format (double, int, int);

// UTF-8 aware.
int characters (const std::string&);

#endif
////////////////////////////////////////////////////////////////////////////////
