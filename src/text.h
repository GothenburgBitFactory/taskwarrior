////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TEXT
#define INCLUDED_TEXT

#include <string>
#include <vector>

// text.cpp, Non-UTF-8 aware.
void wrapText (std::vector <std::string>&, const std::string&, const int, bool);
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
std::string unquoteText (const std::string&);
int longestWord (const std::string&);
int longestLine (const std::string&);
bool extractLine (std::string&, const std::string&, int, bool, unsigned int&);
void splitq (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const std::string&);
void split_minimal (std::vector<std::string>&, const std::string&, const char);
void join (std::string&, const std::string&, const std::vector<std::string>&);
void join (std::string&, const std::string&, const std::vector<int>&);
std::string commify (const std::string&);
std::string lowerCase (const std::string&);
std::string upperCase (const std::string&);
std::string ucFirst (const std::string&);
const std::string str_replace (std::string&, const std::string&, const std::string&);
const std::string str_replace (const std::string&, const std::string&, const std::string&);
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
bool closeEnough (const std::string&, const std::string&, unsigned int minLength = 0);
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
const std::string format (const std::string&, const std::string&, double);
const std::string format (const std::string&, int, const std::string&);
const std::string format (const std::string&, int, int);
const std::string format (const std::string&, int, int, int);
const std::string format (const std::string&, int, double);
const std::string format (const std::string&, const std::string&, const std::string&, const std::string&);

std::string leftJustify (const int, const int);
std::string leftJustify (const std::string&, const int);
std::string rightJustifyZero (const int, const int);
std::string rightJustify (const int, const int);
std::string rightJustify (const std::string&, const int);

int mk_wcwidth (wchar_t);

#endif
////////////////////////////////////////////////////////////////////////////////
