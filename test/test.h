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
#ifndef INCLUDED_UNITTEST
#define INCLUDED_UNITTEST

#include <string>

class UnitTest
{
public:
  UnitTest ();
  UnitTest (int);
  ~UnitTest ();

  void plan (int);
  void planMore (int);
  void ok (bool, const std::string&);
  void notok (bool, const std::string&);
  void is (bool, bool, const std::string&);
  void is (size_t, size_t, const std::string&);
  void is (int, int, const std::string&);
  void is (double, double, const std::string&);
  void is (char, char, const std::string&);
  void is (const std::string&, const std::string&, const std::string&);
  void is (const char*, const char*, const std::string&);
  void diag (const std::string&);
  void pass (const std::string&);
  void fail (const std::string&);
  void skip (const std::string&);

private:
  int mPlanned;
  int mCounter;
  int mPassed;
  int mFailed;
  int mSkipped;
};

#endif

////////////////////////////////////////////////////////////////////////////////
