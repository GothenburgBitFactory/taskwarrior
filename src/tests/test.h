////////////////////////////////////////////////////////////////////////////////
// Copyright 2007, 2008, Paul Beckingham.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_TEST
#define INCLUDED_TEST

#include <string>

void plan (int);
void ok (bool, const std::string&);
void notok (bool, const std::string&);
void is (bool, bool, const std::string&);
void is (int, int, const std::string&);
void is (size_t, size_t, const std::string&);
void is (double, double, const std::string&);
void is (char, char, const std::string&);
void is (const std::string&, const std::string&, const std::string&);
void diag (const std::string&);
void fail (const std::string&);
void pass (const std::string&);

#endif
////////////////////////////////////////////////////////////////////////////////
