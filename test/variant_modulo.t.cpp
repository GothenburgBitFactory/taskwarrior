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

#define EPSILON 0.0001

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (40);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // boolean / boolean  -> ERROR
  try {Variant v00 = v0 % v0; t.fail ("true % true --> error");}
  catch (...)                {t.pass ("true % true --> error");}

  // boolean / integer  -> ERROR
  try {Variant v01 = v0 % v1; t.fail ("true % 42 --> error");}
  catch (...)                {t.pass ("true % 42 --> error");}

  // boolean / real     -> ERROR
  try {Variant v02 = v0 % v2; t.fail ("true % 3.14 --> error");}
  catch (...)                {t.pass ("true % 3.14 --> error");}

  // boolean / string   -> ERROR
  try {Variant v03 = v0 % v3; t.fail ("true % foo --> error");}
  catch (...)                {t.pass ("true % foo --> error");}

  // boolean / date     -> ERROR
  try {Variant v04 = v0 % v4; t.fail ("true % 1234567890 --> error");}
  catch (...)                {t.pass ("true % 1234567890 --> error");}

  // boolean / duration -> ERROR
  try {Variant v05 = v0 % v5; t.fail ("true % 1200 --> error");}
  catch (...)                {t.pass ("true % 1200 --> error");}

  // integer / boolean  -> ERROR
  try {Variant v10 = v1 % v0; t.fail ("42 % true --> error");}
  catch (...)                {t.pass ("42 % true --> error");}

  // integer / integer  -> integer
  Variant v11 = v1 % v1;
  t.is (v11.type (), Variant::type_integer, "42 % 42 --> integer");
  t.is (v11.get_integer (), 0,              "42 % 42 --> 0");

  // integer / real     -> real
  Variant v12 = v1 % v2;
  t.is (v12.type (), Variant::type_real,    "42 % 3.14 --> real");
  t.is (v12.get_real (), 1.18, EPSILON,     "42 % 3.14 --> 1.18");

  // integer / string   -> ERROR
  try {Variant v13 = v1 % v3; t.fail ("42 % foo --> error");}
  catch (...)                {t.pass ("42 % foo --> error");}

  // integer / date     -> ERROR
  try {Variant v14 = v1 % v4; t.fail ("42 % 1234567890 --> error");}
  catch (...)                {t.pass ("42 % 1234567890 --> error");}

  // integer / duration -> duration
  try {Variant v15 = v1 % v5; t.fail ("42 % 1200 --> error");}
  catch (...)                {t.pass ("42 % 1200 --> error");}

  // real / boolean  -> ERROR
  try {Variant v20 = v2 % v0; t.fail ("3.14 % true --> error");}
  catch (...)                {t.pass ("3.14 % true --> error");}

  // real / integer  -> real
  Variant v21 = v2 % v1;
  t.is (v21.type (), Variant::type_real,    "3.14 % 42 --> real");
  t.is (v21.get_real (), 3.14, EPSILON,     "3.14 % 42 --> 3.14");

  // real / real     -> real
  Variant v22 = v2 % v2;
  t.is (v22.type (), Variant::type_real,    "3.14 % 3.14 --> real");
  t.is (v22.get_real (), 0.0, EPSILON,      "3.14 % 3.14 --> 0.0");

  // real / string   -> error
  try {Variant v23 = v2 % v3; t.fail ("3.14 % foo --> error");}
  catch (...)                {t.pass ("3.14 % foo --> error");}

  // real / date     -> error
  try {Variant v24 = v2 % v4; t.fail ("3.14 % 1234567890 --> error");}
  catch (...)                {t.pass ("3.14 % 1234567890 --> error");}

  // real / duration -> duration
  try {Variant v25 = v2 % v5; t.fail ("3.14 % 1200 --> error");}
  catch (...)                {t.pass ("3.14 % 1200 --> error");}

  // string / boolean  -> ERROR
  try {Variant v30 = v3 % v0; t.fail ("foo % true --> error");}
  catch (...)                {t.pass ("foo % true --> error");}

  // string / integer  -> ERROR
  try {Variant v31 = v3 % v1; t.fail ("foo % 42 --> error");}
  catch (...)                {t.pass ("foo % 42 --> error");}

  // string / real     -> ERROR
  try {Variant v32 = v3 % v2; t.fail ("foo % 3.14 --> error");}
  catch (...)                {t.pass ("foo % 3.14 --> error");}

  // string / string   -> ERROR
  try {Variant v33 = v3 % v3; t.fail ("foo % foo --> error");}
  catch (...)                {t.pass ("foo % foo --> error");}

  // string / date     -> ERROR
  try {Variant v34 = v3 % v4; t.fail ("foo % 1234567890 --> error");}
  catch (...)                {t.pass ("foo % 1234567890 --> error");}

  // string / duration -> ERROR
  try {Variant v35 = v3 % v5; t.fail ("foo % 1200 --> error");}
  catch (...)                {t.pass ("foo % 1200 --> error");}

  // date / boolean  -> ERROR
  try {Variant v40 = v4 % v0; t.fail ("1234567890 % true --> error");}
  catch (...)                {t.pass ("1234567890 % true --> error");}

  // date / integer  -> ERROR
  try {Variant v41 = v4 % v1; t.fail ("1234567890 % 42 --> error");}
  catch (...)                {t.pass ("1234567890 % 42 --> error");}

  // date / real     -> ERROR
  try {Variant v42 = v4 % v2; t.fail ("1234567890 % 3.14 --> error");}
  catch (...)                {t.pass ("1234567890 % 3.14 --> error");}

  // date / string   -> ERROR
  try {Variant v43 = v4 % v3; t.fail ("1234567890 % foo --> error");}
  catch (...)                {t.pass ("1234567890 % foo --> error");}

  // date / date     -> ERROR
  try {Variant v44 = v4 % v4; t.fail ("1234567890 % 1234567890 --> error");}
  catch (...)                {t.pass ("1234567890 % 1234567890 --> error");}

  // date / duration -> ERROR
  try {Variant v45 = v4 % v5; t.fail ("1234567890 % 1200 --> error");}
  catch (...)                {t.pass ("1234567890 % 1200 --> error");}

  // duration / boolean  -> ERROR
  try {Variant v50 = v5 % v0; t.fail ("1200 % true --> error");}
  catch (...)                {t.pass ("1200 % true --> error");}

  // duration / integer  -> ERROR
  try {Variant v51 = v5 % v1; t.fail ("1200 % 42 --> error");}
  catch (...)                {t.pass ("1200 % 42 --> error");}

  // duration / real     -> ERROR
  try {Variant v52 = v5 % v2; t.fail ("1200 % 3.14 --> error");}
  catch (...)                {t.pass ("1200 % 3.14 --> error");}

  // duration / string   -> ERROR
  try {Variant v53 = v5 % v3; t.fail ("1200 % foo --> error");}
  catch (...)                {t.pass ("1200 % foo --> error");}

  // duration / date     -> ERROR
  try {Variant v54 = v5 % v4; t.fail ("1200 % 1234567890 --> error");}
  catch (...)                {t.pass ("1200 % 1234567890 --> error");}

  // duration / duration -> ERROR
  try {Variant v55 = v5 % v5; t.fail ("1200 % 1200 --> error");}
  catch (...)                {t.pass ("1200 % 1200 --> error");}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
