////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
  UnitTest t (18);

  // void set (const std::string&, const int);
  // int get (const std::string&, const int);
  Config c;
  c.set ("int1", 0);
  t.is (c.get ("int1", 9), 0, "Config::set/get int");

  c.set ("int2", 3);
  t.is (c.get ("int2", 9), 3, "Config::set/get int");

  c.set ("int3", -9);
  t.is (c.get ("int3", 9), -9, "Config::set/get int");

  // void set (const std::string&, const double);
  // double get (const std::string&, const double);
  c.set ("double1", 0.0);
  t.is (c.get ("double1", 9.0), 0.0, "Config::set/get double");

  c.set ("double2", 3.0);
  t.is (c.get ("double2", 9.0), 3.0, "Config::set/get double");

  c.set ("double3", -9.0);
  t.is (c.get ("double3", 9.0), -9.0, "Config::set/get double");

  // void set (const std::string&, const std::string&);
  c.set ("str1", "one");
  t.is (c.get ("str1", ""), "one", "Config::set/get std::string");

  c.set ("str1", "");
  t.is (c.get ("str1", "no"), "", "Config::set/get std::string");

  // const std::string get (const char*);
  c.set ("str1", "one");
  t.is (c.get ((char*) "str1"), (char*)"one", "Config::set/get char*");

  // const std::string get (const char*, const char*);
  c.set ("str1", "one");
  t.is (c.get ((char*)"str1", (char*)""), "one", "Config::set/get char*");

  c.set ("str1", "");
  t.is (c.get ((char*)"str1", (char*)"no"), "", "Config::set/get char*");

  // const std::string get (const std::string&);
  c.set ("str1", "one");
  t.is (c.get (std::string ("str1")), "one", "Config::set/get std::string");

  c.set ("str1", "");
  t.is (c.get (std::string ("str1")), "", "Config::set/get std::string");

  // const std::string get (const std::string&, const std::string&);
  c.set ("str1", "one");
  t.is (c.get (std::string ("str1"), std::string ("no")), "one", "Config::set/get std::string");

  c.set ("str1", "");
  t.is (c.get (std::string ("str1"), std::string ("no")), "", "Config::set/get std::string");

  // bool get (const std::string&, const bool);
  c.set ("bool1", false);
  t.is (c.get (std::string ("bool1"), (bool)true), false, "Config::set/get bool");

  c.set ("bool1", true);
  t.is (c.get (std::string ("bool1"), (bool)false), true, "Config::set/get bool");

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
