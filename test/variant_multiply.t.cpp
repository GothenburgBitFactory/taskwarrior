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
  UnitTest t (54);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // boolean * boolean  -> ERROR
  try {Variant v00 = v0 * v0; t.fail ("true * true --> error");}
  catch (...)                {t.pass ("true * true --> error");}

  // boolean * integer  -> integer
  Variant v01 = v0 * v1;
  t.is (v01.type (), Variant::type_integer, "true * 42 --> integer");
  t.is (v01.get_integer (), 42,             "true * 42 --> 42");

  // boolean * real     -> real
  Variant v02 = v0 * v2;
  t.is (v02.type (), Variant::type_real,    "true * 3.14 --> real");
  t.is (v02.get_real (), 3.14, EPSILON,     "true * 3.14 --> 3.14");

  // boolean * string   -> string
  Variant v03 = v0 * v3;
  t.is (v03.type (), Variant::type_string,  "true * foo --> real");
  t.is (v03.get_string (), "foo",           "true * foo --> foo");

  // boolean * date     -> ERROR
  try {Variant v04 = v0 * v4; t.fail ("true * 1234567890 --> error");}
  catch (...)                {t.pass ("true * 1234567890 --> error");}

  // boolean * duration -> duration
  Variant v05 = v0 * v5;
  t.is (v05.type (), Variant::type_duration, "true * 1200 --> duration");
  t.is (v05.get_duration (), 1200,           "true * 1200 --> 1200");

  // integer * boolean  -> integer
  Variant v10 = v1 * v0;
  t.is (v10.type (), Variant::type_integer, "42 * true --> integer");
  t.is (v10.get_integer (), 42,             "42 * true --> 42");

  // integer * integer  -> integer
  Variant v11 = v1 * v1;
  t.is (v11.type (), Variant::type_integer, "42 * 42 --> integer");
  t.is (v11.get_integer (), 1764,           "42 * 42 --> 1764");

  // integer * real     -> real
  Variant v12 = v1 * v2;
  t.is (v12.type (), Variant::type_real,    "42 * 3.14 --> real");
  t.is (v12.get_real (), 131.88, EPSILON,   "42 * 3.14 --> 131.88");

  // integer * string   -> string
  Variant v13 = v1 * v3;
  t.is (v13.type (), Variant::type_string,  "42 * foo --> string");
  t.is (v13.get_string ().substr (0, 10), "foofoofoof",
                                            "42 * foo --> foofoofoofoo...");
  // integer * date     -> error
  try {Variant v14 = v1 * v4; t.fail ("42 * 1234567890 --> error");}
  catch (...)                {t.pass ("42 * 1234567890 --> error");}

  // integer * duration -> duration
  Variant v15 = v1 * v5;
  t.is (v15.type (), Variant::type_duration, "42 * 1200 --> duration");
  t.is (v15.get_duration (), 50400,          "42 * 1200 --> 50400");

  // real * boolean  -> real
  Variant v20 = v2 * v0;
  t.is (v20.type (), Variant::type_real,    "3.14 * true --> real");
  t.is (v20.get_real (), 3.14, EPSILON,     "3.14 * true --> 3.14");

  // real * integer  -> real
  Variant v21 = v2 * v1;
  t.is (v21.type (), Variant::type_real,    "3.14 * 42 --> real");
  t.is (v21.get_real (), 131.88, EPSILON,   "3.14 * 42 --> 131.88");

  // real * real     -> real
  Variant v22 = v2 * v2;
  t.is (v22.type (), Variant::type_real,    "3.14 * 3.14 --> real");
  t.is (v22.get_real (), 9.8596, EPSILON,   "3.14 * 3.14 --> 9.8596");

  // real * string   -> error
  try {Variant v23 = v2 * v3; t.fail ("3.14 * foo --> error");}
  catch (...)                {t.pass ("3.14 * foo --> error");}

  // real * date     -> error
  try {Variant v24 = v2 * v4; t.fail ("3.14 * 1234567890 --> error");}
  catch (...)                {t.pass ("3.14 * 1234567890 --> error");}

  // real * duration -> duration
  Variant v25 = v2 * v5;
  t.is (v25.type (), Variant::type_duration, "3.14 * 1200 --> duration");
  t.is (v25.get_duration (), 3768,           "3.14 * 1200 --> 3768");

  // string * boolean  -> string
  Variant v30 = v3 * v0;
  t.is (v30.type (), Variant::type_string,  "foo * true --> real");
  t.is (v30.get_string (), "foo",           "foo * true --> foo");

  // string * integer  -> string
  Variant v31 = v3 * v1;
  t.is (v31.type (), Variant::type_string,  "foo * 42 --> string");
  t.is (v31.get_string ().substr (0, 10), "foofoofoof",
                                            "foo * 42 --> foofoofoof...");

  // string * real     -> string
  try {Variant v32 = v3 * v2; t.fail ("foo * 3.14 --> error");}
  catch (...)                {t.pass ("foo * 3.14 --> error");}

  // string * string   -> string
  try {Variant v33 = v3 * v3; t.fail ("foo * foo --> error");}
  catch (...)                {t.pass ("foo * foo --> error");}

  // string * date     -> string
  try {Variant v34 = v3 * v4; t.fail ("foo * 1234567890 --> error");}
  catch (...)                {t.pass ("foo * 1234567890 --> error");}

  // string * duration -> string
  try {Variant v35 = v3 * v5; t.fail ("foo * 1200 --> error");}
  catch (...)                {t.pass ("foo * 1200 --> error");}

  // date * boolean  -> ERROR
  try {Variant v40 = v4 * v0; t.fail ("1234567890 * true --> error");}
  catch (...)                {t.pass ("1234567890 * true --> error");}

  // date * integer  -> ERROR
  try {Variant v41 = v4 * v1; t.fail ("1234567890 * 42 --> error");}
  catch (...)                {t.pass ("1234567890 * 42 --> error");}

  // date * real     -> ERROR
  try {Variant v42 = v4 * v2; t.fail ("1234567890 * 3.14 --> error");}
  catch (...)                {t.pass ("1234567890 * 3.14 --> error");}

  // date * string   -> ERROR
  try {Variant v43 = v4 * v3; t.fail ("1234567890 * foo --> error");}
  catch (...)                {t.pass ("1234567890 * foo --> error");}

  // date * date     -> ERROR
  try {Variant v44 = v4 * v4; t.fail ("1234567890 * 1234567890 --> error");}
  catch (...)                {t.pass ("1234567890 * 1234567890 --> error");}

  // date * duration -> ERROR
  try {Variant v45 = v4 * v5; t.fail ("1234567890 * 1200 --> error");}
  catch (...)                {t.pass ("1234567890 * 1200 --> error");}

  // duration * boolean  -> duration
  Variant v50 = v5 * v0;
  t.is (v50.type (), Variant::type_duration, "1200 * true --> duration");
  t.is (v50.get_duration (), 1200,           "1200 * true --> 1200");

  // duration * integer  -> duration
  Variant v51 = v5 * v1;
  t.is (v51.type (), Variant::type_duration, "1200 * 42 --> duration");
  t.is (v51.get_duration (), 50400,          "1200 * 42 --> 50400");

  // duration * real     -> duration
  Variant v52 = v5 * v2;
  t.is (v52.type (), Variant::type_duration, "1200 * 3.14 --> duration");
  t.is (v52.get_duration (), 3768,           "1200 * 3.14 --> 3768");

  // duration * string   -> string
  try {Variant v53 = v5 * v3; t.fail ("1200 * foo --> error");}
  catch (...)                {t.pass ("1200 * foo --> error");}

  // duration * date     -> date
  try {Variant v54 = v5 * v4; t.fail ("1200 * 1234567890 --> error");}
  catch (...)                {t.pass ("1200 * 1234567890 --> error");}

  // duration * duration -> duration
  try {Variant v55 = v5 * v5; t.fail ("1200 * 1200 --> error");}
  catch (...)                {t.pass ("1200 * 1200 --> error");}

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
