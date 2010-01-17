////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <Context.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (11);

  Config c;

  // void set (const std::string&, const std::string&);
  // const std::string get (const std::string&);
  c.set ("str1", "one");
  t.is (c.get ("str1"), "one", "Config::set/get std::string");

  c.set ("str1", "");
  t.is (c.get ("str1"), "", "Config::set/get std::string");

  // void set (const std::string&, const int);
  // const int getInteger (const std::string&);
  c.set ("int1", 1);
  t.is (c.getInteger ("int1"), 1, "Config::set/get int");

  c.set ("int2", 3);
  t.is (c.getInteger ("int2"), 3, "Config::set/get int");

  c.set ("int3", -9);
  t.is (c.getInteger ("int3"), -9, "Config::set/get int");

  // void set (const std::string&, const double);
  // const double getReal (const std::string&);
  c.set ("double1", 1.0);
  t.is (c.getReal ("double1"), 1.0, "Config::set/get double");

  c.set ("double2", 3.0);
  t.is (c.getReal ("double2"), 3.0, "Config::set/get double");

  c.set ("double3", -9.0);
  t.is (c.getReal ("double3"), -9.0, "Config::set/get double");

  // void set (const std::string&, const bool);
  // const bool getBoolean (const std::string&);
  c.set ("bool1", false);
  t.is (c.getBoolean ("bool1"), false, "Config::set/get bool");

  c.set ("bool1", true);
  t.is (c.getBoolean ("bool1"), true, "Config::set/get bool");

  // void all (std::vector <std::string>&);
  std::vector <std::string> all;
  c.all (all);

  // 8 created in this test program.
  // 22 default report setting created in Config::Config.
  t.ok (all.size () >= 8, "Config::all");

  // TODO Test includes
  // TODO Test included nesting limit
  // TODO Test included absolute vs relative
  // TODO Test included missing file

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
