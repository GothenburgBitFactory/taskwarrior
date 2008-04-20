////////////////////////////////////////////////////////////////////////////////
// Copyright 2004 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_LIBRARY
#define INCLUDED_LIBRARY

#include <string>
#include <vector>
#include <sys/types.h>

#include "stlmacros.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

// text.cpp
void wrapText (std::vector <std::string>&, const std::string&, const int);
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
std::wstring trimLeft (const std::wstring& in, const std::wstring& t = L" ");  // UNICODE safe
std::wstring trimRight (const std::wstring& in, const std::wstring& t = L" "); // UNICODE safe
std::wstring trim (const std::wstring& in, const std::wstring& t = L" ");      // UNICODE safe
void extractParagraphs (const std::string&, std::vector<std::string>&);
void extractLine (std::string&, std::string&, int);
void split (std::vector<std::string>&, const std::string&, const char);
void split (std::vector<std::string>&, const std::string&, const std::string&);
void join (std::string&, const std::string&, const std::vector<std::string>&);
std::string commify (const std::string&);
std::string lowerCase (const std::string&);

// misc.cpp
void delay (float);

// list.cpp
int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&);

// units.cpp
void formatTimeDeltaDays (std::string&, time_t);
std::string formatSeconds (time_t);

// uuid.cpp
const std::string uuid ();

#endif
////////////////////////////////////////////////////////////////////////////////
