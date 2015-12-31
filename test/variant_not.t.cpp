////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <test.h>
#include <Variant.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (14);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // Truth table.
  Variant vFalse (false);
  Variant vTrue (true);
  t.is (!vFalse, true,  "!false --> true");
  t.is (!vTrue,  false, "!true --> false");

  Variant v00 = ! v0;
  t.is (v00.type (), Variant::type_boolean, "! true --> boolean");
  t.is (v00.get_bool (), false,             "! true --> false");

  Variant v01 = ! v1;
  t.is (v01.type (), Variant::type_boolean, "! 42 --> boolean");
  t.is (v01.get_bool (), false,             "! 42 --> false");

  Variant v02 = ! v2;
  t.is (v02.type (), Variant::type_boolean, "! 3.14 --> boolean");
  t.is (v02.get_bool (), false,             "! 3.14 --> false");

  Variant v03 = ! v3;
  t.is (v03.type (), Variant::type_boolean, "! foo --> boolean");
  t.is (v03.get_bool (), false,             "! foo --> false");

  Variant v04 = ! v4;
  t.is (v04.type (), Variant::type_boolean, "! 1234567890 --> boolean");
  t.is (v04.get_bool (), false,             "! 1234567890 --> false");

  Variant v05 = ! v5;
  t.is (v05.type (), Variant::type_boolean, "! 1200 --> boolean");
  t.is (v05.get_bool (), false,             "! 1200 --> false");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
