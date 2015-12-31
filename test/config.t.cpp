////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>
#include <iostream>
#include <stdlib.h>
#include <Context.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (11);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

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
