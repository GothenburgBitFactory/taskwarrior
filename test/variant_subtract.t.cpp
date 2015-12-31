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


#define EPSILON 0.001

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (55);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // boolean - boolean  -> ERROR
  try {Variant v00 = v0 - v0; t.fail ("true - true --> error");}
  catch (...)                {t.pass ("true - true --> error");}

  // boolean - integer  -> ERROR
  try {Variant v01 = v0 - v1; t.fail ("true - 42 --> error");}
  catch (...)                {t.pass ("true - 42 --> error");}

  // boolean - real     -> ERROR
  try {Variant v02 = v0 - v2; t.fail ("true - 3.14 --> error");}
  catch (...)                {t.pass ("true - 3.14 --> error");}

  // boolean - string   -> ERROR
  try {Variant v03 = v0 - v3; t.fail ("true - foo --> error");}
  catch (...)                {t.pass ("true - foo --> error");}

  // boolean - date     -> ERROR
  try {Variant v04 = v0 - v4; t.fail ("true - 1234567890 --> error");}
  catch (...)                {t.pass ("true - 1234567890 --> error");}

  // boolean - duration -> ERROR
  try {Variant v05 = v0 - v5; t.fail ("true - 1200 --> error");}
  catch (...)                {t.pass ("true - 1200 --> error");}

  // integer - boolean  -> integer
  Variant v10 = v1 - v0;
  t.is (v10.type (), Variant::type_integer, "42 - true --> integer");
  t.is (v10.get_integer (), 41,             "42 - true --> 41");

  // integer - integer  -> integer
  Variant v11 = v1 - v1;
  t.is (v11.type (), Variant::type_integer, "42 - 42 --> integer");
  t.is (v11.get_integer (), 0,              "42 - 42 --> 0");

  // integer - real     -> real
  Variant v12 = v1 - v2;
  t.is (v12.type (), Variant::type_real,    "42 - 3.14 --> real");
  t.is (v12.get_real (), 38.86, EPSILON,    "42 - 3.14 --> 38.86");

  // integer - string   -> ERROR
  try {Variant v13 = v1 - v3; t.fail ("42 - foo --> error");}
  catch (...)                {t.pass ("42 - foo --> error");}

  // integer - date     -> date
  Variant v1a (1300000000);
  Variant v14 = v1a - v4;
  t.is (v14.type (), Variant::type_date,    "1300000000 - 1234567890 --> date");
  t.is (v14.get_date (), 65432110,          "1300000000 - 1234567890 --> 65432110");

  // integer - duration -> duration
  Variant v15 = v1a - v5;
  t.is (v15.type (), Variant::type_duration, "1300000000 - 1200 --> duration");
  t.is (v15.get_duration (), 1299998800,     "1300000000 - 1200 --> 1299998800");

  // real - boolean  -> real
  Variant v20 = v2 - v0;
  t.is (v20.type (), Variant::type_real,    "3.14 - true --> real");
  t.is (v20.get_real (), 2.14, EPSILON,     "3.14 - true --> 2.14");

  // real - integer  -> real
  Variant v21 = v2 - v1;
  t.is (v21.type (), Variant::type_real,    "3.14 - 42 --> real");
  t.is (v21.get_real (), -38.86, EPSILON,   "3.14 - 42 --> -38.86");

  // real - real     -> real
  Variant v22 = v2 - v2;
  t.is (v22.type (), Variant::type_real,    "3.14 - 3.14 --> real");
  t.is (v22.get_real (), 0.0, EPSILON,      "3.14 - 3.14 --> 0.0");

  // real - string   -> ERROR
  try {Variant v23 = v1 - v3; t.fail ("3.14 - foo --> error");}
  catch (...)                {t.pass ("3.14 - foo --> error");}

  // real - date     -> real
  Variant v2a (1300000000.0);
  Variant v24 = v2a - v4;
  t.is (v24.type (), Variant::type_real,    "1300000000.0 - 1234567890 --> real");
  t.is (v24.get_real (), 65432110.0,        "1300000000.0 - 1234567890 --> 65432110");

  // real - duration -> real
  Variant v25 = v2a - v5;
  t.is (v25.type (), Variant::type_real,    "1300000000.0 - 1200 --> real");
  t.is (v25.get_real (), 1299998800.0,      "1300000000.0 - 1200 --> 1299998800");

  // string - boolean  -> ERROR
  try {Variant v30 = v3 - v0; t.fail ("foo - foo --> error");}
  catch (...)                {t.pass ("foo - foo --> error");}

  // string - integer  -> ERROR
  try {Variant v31 = v3 - v1; t.fail ("foo - 42 --> error");}
  catch (...)                {t.pass ("foo - 42 --> error");}

  // string - real     -> ERROR
  try {Variant v32 = v3 - v2; t.fail ("foo - 3.14 --> error");}
  catch (...)                {t.pass ("foo - 3.14 --> error");}

  // string - string   -> ERROR
  try {Variant v33 = v3 - v3; t.fail ("foo - foo --> error");}
  catch (...)                {t.pass ("foo - foo --> error");}

  // string - date     -> ERROR
  try {Variant v34 = v3 - v4; t.fail ("foo - 1234567890 --> error");}
  catch (...)                {t.pass ("foo - 1234567890 --> error");}

  // string - duration -> ERROR
  try {Variant v35 = v3 - v5; t.fail ("foo - 1200 --> error");}
  catch (...)                {t.pass ("foo - 1200 --> error");}

  // date - boolean  -> date
  Variant v40 = v4 - v0;
  t.is (v40.type (), Variant::type_date,     "1234567890 - true --> date");
  t.is (v40.get_date (), 1234567889,         "1234567890 - true --> 1234567889");

  // date - integer  -> date
  Variant v41 = v4 - v1;
  t.is (v41.type (), Variant::type_date,     "1234567890 - 42 --> date");
  t.is (v41.get_date (), 1234567848,         "1234567890 - 42 --> 1234567848");

  // date - real     -> date
  Variant v42 = v4 - v2;
  t.is (v42.type (), Variant::type_date,     "1234567890 - 3.14 --> date");
  t.is (v42.get_date (), 1234567887,         "1234567890 - 3.14 --> 1234567887");

  // date - string   -> string
  try {Variant v43 = v4 - v3; t.fail ("1234567890 - foo --> error");}
  catch (...)                {t.pass ("1234567890 - foo --> error");}

  // date - date     -> duration
  Variant v44 = v4 - v4;
  t.is (v44.type (), Variant::type_duration, "1234567890 - 1234567890 --> duration");
  t.is (v44.get_duration (), 0,              "1234567890 - 1234567890 --> 0");

  // date - duration -> date
  Variant v45 = v4 - v5;
  t.is (v45.type (), Variant::type_date,   "1234567890 - 1200 --> date");
  t.is (v45.get_date (), 1234566690,       "1234567890 - 1200 --> 1234566690");

  // duration - boolean  -> duration
  Variant v50 = v5 - v0;
  t.is (v50.type (), Variant::type_duration, "1200 - true --> duration");
  t.is (v50.get_duration (), 1199,           "1200 - true --> 1199");

  // duration - integer  -> duration
  Variant v51 = v5 - v1;
  t.is (v51.type (), Variant::type_duration, "1200 - 42 --> duration");
  t.is (v51.get_duration (), 1158,           "1200 - 42 --> 1158");

  // duration - real     -> duration
  Variant v52 = v5 - v2;
  t.is (v52.type (), Variant::type_duration, "1200 - 3.14 --> duration");
  t.is (v52.get_duration (), 1197,           "1200 - 3.14 --> 1197");

  // duration - string   -> ERROR
  try {Variant v53 = v5 - v3; t.fail ("1200 - foo --> error");}
  catch (...)                {t.pass ("1200 - foo --> error");}

  // duration - date     -> ERROR
  try {Variant v54 = v5 - v4; t.fail ("1200 - 1234567890 --> error");}
  catch (...)                {t.pass ("1200 - 1234567890 --> error");}

  // duration - duration -> duration
  Variant v55 = v5 - v5;
  t.is (v55.type (), Variant::type_duration, "1200 - 1200 --> duration");
  t.is (v55.get_duration (), 0,              "1200 - 1200 --> 0");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
