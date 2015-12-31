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
  UnitTest t (80);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // boolean + boolean  -> ERROR
  try {Variant v00 = v0 + v0; t.fail ("true + true --> error");}
  catch (...)                {t.pass ("true + true --> error");}

  // boolean + integer  -> integer
  Variant v01 = v0 + v1;
  t.is (v01.type (), Variant::type_integer, "true + 42 --> integer");
  t.is (v01.get_integer (), 43,             "true + 42 --> 43");

  // boolean + real     -> real
  Variant v02 = v0 + v2;
  t.is (v02.type (), Variant::type_real,    "true + 3.14 --> real");
  t.is (v02.get_real (), 4.14, EPSILON,     "true + 3.14 --> 4.14");

  // boolean + string   -> string
  Variant v03 = v0 + v3;
  t.is (v03.type (), Variant::type_string,  "true + foo --> string");
  t.is (v03.get_string (), "truefoo",       "true + foo --> truefoo");

  // boolean + date     -> date
  Variant v04 = v0 + v4;
  t.is (v04.type (), Variant::type_date,    "true + 1234567890 --> date");
  t.is (v04.get_date (), "1234567891",      "true + 1234567890 --> 1234567891");

  // boolean + duration -> duration
  Variant v05 = v0 + v5;
  t.is (v05.type (), Variant::type_duration, "true + 1200 --> duration");
  t.is (v05.get_duration (), "1201",         "true + 1200 --> 1201");

  // integer + boolean  -> integer
  Variant v10 = v1 + v0;
  t.is (v10.type (), Variant::type_integer, "42 + true --> integer");
  t.is (v10.get_integer (), 43,             "42 + true --> 43");

  // integer + integer  -> integer
  Variant v11 = v1 + v1;
  t.is (v11.type (), Variant::type_integer, "42 + 42 --> integer");
  t.is (v11.get_integer (), 84,             "42 + 42 --> 84");

  // integer + real     -> real
  Variant v12 = v1 + v2;
  t.is (v12.type (), Variant::type_real,    "42 + 3.14 --> real");
  t.is (v12.get_real (), 45.14, EPSILON,    "42 + 3.14 --> 45.14");

  // integer + string   -> string
  Variant v13 = v1 + v3;
  t.is (v13.type (), Variant::type_string,  "42 + foo --> string");
  t.is (v13.get_string (), "42foo",         "42 + foo --> 42foo");

  // integer + date     -> date
  Variant v14 = v1 + v4;
  t.is (v14.type (), Variant::type_date,    "42 + 1234567890 --> date");
  t.is (v14.get_date (), 1234567932,        "42 + 1234567890 --> 1234567932");

  // integer + duration -> duration
  Variant v15 = v1 + v5;
  t.is (v15.type (), Variant::type_duration, "42 + 1200 --> duration");
  t.is (v15.get_duration (), 1242,           "42 + 1200 --> 1242");

  // real + boolean  -> real
  Variant v20 = v2 + v0;
  t.is (v20.type (), Variant::type_real,    "3.14 + true --> real");
  t.is (v20.get_real (), 4.14, EPSILON,     "3.14 + true --> 4.14");

  // real + integer  -> real
  Variant v21 = v2 + v1;
  t.is (v21.type (), Variant::type_real,    "3.14 + 42 --> real");
  t.is (v21.get_real (), 45.14, EPSILON,    "3.14 + 42 --> 45.14");

  // real + real     -> real
  Variant v22 = v2 + v2;
  t.is (v22.type (), Variant::type_real,    "3.14 + 3.14 --> real");
  t.is (v22.get_real (), 6.28, EPSILON,     "3.14 + 3.14 --> 6.28");

  // real + string   -> string
  Variant v23 = v2 + v3;
  t.is (v23.type (), Variant::type_string,  "3.14 + foo --> string");
  t.is (v23.get_string (), "3.14foo",       "3.14 + foo --> 3.14foo");

  // real + date     -> date
  Variant v24 = v2 + v4;
  t.is (v24.type (), Variant::type_date,    "3.14 + 1234567890 --> date");
  t.is (v24.get_date (), 1234567893,        "3.14 + 1234567890 --> 1234567893");

  // real + duration -> duration
  Variant v25 = v2 + v5;
  t.is (v25.type (), Variant::type_duration, "3.14 + 1200 --> duration");
  t.is (v25.get_duration (), 1203,           "3.14 + 1200 --> 1203");

  // string + boolean  -> string
  Variant v30 = v3 + v0;
  t.is (v30.type (), Variant::type_string,   "foo + true --> string");
  t.is (v30.get_string (), "footrue",        "foo + true --> footrue");

  // string + integer  -> string
  Variant v31 = v3 + v1;
  t.is (v31.type (), Variant::type_string,   "foo + 42 --> string");
  t.is (v31.get_string (), "foo42",          "foo + 42 --> foo42");

  // string + real     -> string
  Variant v32 = v3 + v2;
  t.is (v32.type (), Variant::type_string,   "foo + 3.14 --> string");
  t.is (v32.get_string (), "foo3.14",        "foo + 3.14 --> foo3.14");

  // string + string   -> string
  Variant v33 = v3 + v3;
  t.is (v33.type (), Variant::type_string,   "foo + foo --> string");
  t.is (v33.get_string (), "foofoo",         "foo + foo --> foofoo");

  // string + date     -> string
  Variant v34 = v3 + v4;
  t.is (v34.type (), Variant::type_string,   "foo + 1234567890 --> string");
  std::string s = v34.get_string ();
  t.is ((int)s[7],  (int)'-',                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");
  t.is ((int)s[10], (int)'-',                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");
  t.is ((int)s[13], (int)'T',                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");
  t.is ((int)s[16], (int)':',                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");
  t.is ((int)s[19], (int)':',                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");
  t.is ((int)s.length (), 22,                "foo + 1234567890 --> fooYYYY-MM-DDThh:mm:ss");

  // string + duration -> string
  Variant v35 = v3 + v5;
  t.is (v35.type (), Variant::type_string,   "foo + 1200 --> string");
  t.is (v35.get_string (), "fooPT20M",       "foo + 1200 --> fooPT20M");

  // date + boolean  -> date
  Variant v40 = v4 + v0;
  t.is (v40.type (), Variant::type_date,     "1234567890 + true --> date");
  t.is (v40.get_date (), 1234567891,         "1234567890 + true --> 1234567891");

  // date + integer  -> date
  Variant v41 = v4 + v1;
  t.is (v41.type (), Variant::type_date,     "1234567890 + 42 --> date");
  t.is (v41.get_date (), 1234567932,         "1234567890 + 42 --> 1234567932");

  // date + real     -> date
  Variant v42 = v4 + v2;
  t.is (v42.type (), Variant::type_date,     "1234567890 + 3.14 --> date");
  t.is (v42.get_date (), 1234567893,         "1234567890 + 3.14 --> 1234567893");

  // date + string   -> string
  Variant v43 = v4 + v3;
  t.is (v43.type (), Variant::type_string,   "1234567890 + foo --> string");
  s = v43.get_string ();
  t.is ((int)s[4],  (int)'-',                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");
  t.is ((int)s[7],  (int)'-',                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");
  t.is ((int)s[10], (int)'T',                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");
  t.is ((int)s[13], (int)':',                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");
  t.is ((int)s[16], (int)':',                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");
  t.is ((int)s.length (), 22,                "1234567890 + foo --> YYYY-MM-DDThh:mm:ssfoo");

  // date + date     -> ERROR
  try {Variant v44 = v4 + v4; t.fail ("1234567890 + 1234567890 --> error");}
  catch (...)                {t.pass ("1234567890 + 1234567890 --> error");}

  // date + duration -> date
  Variant v45 = v4 + v5;
  t.is (v45.type (), Variant::type_date,   "1234567890 + 1200 --> date");
  t.is (v45.get_date (), 1234569090,  "1234567890 + 1200 --> 1234569090");

  // duration + boolean  -> duration
  Variant v50 = v5 + v0;
  t.is (v50.type (), Variant::type_duration, "1200 + true --> duration");
  t.is (v50.get_duration (), 1201,           "1200 + true --> 1201");

  // duration + integer  -> duration
  Variant v51 = v5 + v1;
  t.is (v51.type (), Variant::type_duration, "1200 + 42 --> duration");
  t.is (v51.get_duration (), 1242,           "1200 + 42 --> 1242");

  // duration + real     -> duration
  Variant v52 = v5 + v2;
  t.is (v52.type (), Variant::type_duration, "1200 + 3.14 --> duration");
  t.is (v52.get_duration (), 1203,           "1200 + 3.14 --> 1203");

  // duration + string   -> string
  Variant v53 = v5 + v3;
  t.is (v53.type (), Variant::type_string,   "1200 + foo --> string");
  t.is (v53.get_string (), "PT20Mfoo",       "1200 + foo --> PT20Mfoo");

  // duration + date     -> date
  Variant v54 = v5 + v4;
  t.is (v54.type (), Variant::type_date, "1200 + 1234567890 --> date");
  t.is (v54.get_date (), 1234569090,     "1200 + 1234567890 --> 1234569090");

  // duration + duration -> duration
  Variant v55 = v5 + v5;
  t.is (v55.type (), Variant::type_duration, "1200 + 1200 --> duration");
  t.is (v55.get_duration (), 2400,           "1200 + 1200 --> 2400");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
