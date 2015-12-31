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
  UnitTest t (81);

  try
  {
    // Variant::type_boolean --> *
    Variant v00 (true);
    v00.cast (Variant::type_boolean);
    t.ok (v00.type () == Variant::type_boolean, "cast boolean --> boolean");
    t.ok (v00.get_bool () == true,              "cast boolean --> boolean");

    Variant v01 (true);
    v01.cast (Variant::type_integer);
    t.ok (v01.type () == Variant::type_integer, "cast boolean --> integer");
    t.ok (v01.get_integer () == 1,              "cast boolean --> integer");

    Variant v02 (true);
    v02.cast (Variant::type_real);
    t.ok (v02.type () == Variant::type_real,    "cast boolean --> real");
    t.is (v02.get_real (), 1.0, EPSILON,       "cast boolean --> real");

    Variant v03 (true);
    v03.cast (Variant::type_string);
    t.ok (v03.type () == Variant::type_string,  "cast boolean --> string");
    t.ok (v03.get_string () == "true",          "cast boolean --> string");

    Variant v04 (true);
    v04.cast (Variant::type_date);
    t.ok (v04.type () == Variant::type_date,    "cast boolean --> date");
    t.is (v04.get_date (), 1,                   "cast boolean --> date");

    Variant v05 (true);
    v05.cast (Variant::type_duration);
    t.ok (v05.type () == Variant::type_duration, "cast boolean --> duration");
    t.is (v05.get_duration (), 1,                "cast boolean --> duration");

    // Variant::type_integer --> *
    Variant v10 (42);
    v10.cast (Variant::type_boolean);
    t.ok (v10.type () == Variant::type_boolean, "cast integer --> boolean");
    t.ok (v10.get_bool () == true,              "cast integer --> boolean");

    Variant v11 (42);
    v11.cast (Variant::type_integer);
    t.ok (v11.type () == Variant::type_integer, "cast integer --> integer");
    t.ok (v11.get_integer () == 42,             "cast integer --> integer");

    Variant v12 (42);
    v12.cast (Variant::type_real);
    t.ok (v12.type () == Variant::type_real,    "cast integer --> real");
    t.is (v12.get_real (), 42.0, EPSILON,       "cast integer --> real");

    Variant v13 (42);
    v13.cast (Variant::type_string);
    t.is (v13.type (), Variant::type_string,    "cast integer --> string");
    t.is (v13.get_string (), "42",              "cast integer --> string");

    Variant v14 (42);
    v14.cast (Variant::type_date);
    t.ok (v14.type () == Variant::type_date,    "cast integer --> date");
    t.ok (v14.get_date () == 42,                "cast integer --> date");

    Variant v15 (42);
    v15.cast (Variant::type_duration);
    t.is (v15.type (), Variant::type_duration,  "cast integer --> duration");
    t.is (v15.get_duration (), 42,              "cast integer --> duration");

    // Variant::type_real --> *
    Variant v20 (3.14);
    v20.cast (Variant::type_boolean);
    t.ok (v20.type () == Variant::type_boolean, "cast real --> boolean");
    t.ok (v20.get_bool () == true,              "cast real --> boolean");

    Variant v21 (3.14);
    v21.cast (Variant::type_integer);
    t.ok (v21.type () == Variant::type_integer, "cast real --> integer");
    t.ok (v21.get_integer () == 3,              "cast real --> integer");

    Variant v22 (3.14);
    v22.cast (Variant::type_real);
    t.ok (v22.type () == Variant::type_real,    "cast real --> real");
    t.is (v22.get_real (), 3.14, EPSILON,       "cast real --> real");

    Variant v23 (3.14);
    v23.cast (Variant::type_string);
    t.ok (v23.type () == Variant::type_string,  "cast real --> string");
    t.ok (v23.get_string () == "3.14",          "cast real --> string");

    Variant v24 (3.14);
    v24.cast (Variant::type_date);
    t.ok (v24.type () == Variant::type_date,     "cast real --> date");
    t.ok (v24.get_date () == 3,                  "cast real --> date");

    Variant v25 (3.14);
    v25.cast (Variant::type_duration);
    t.ok (v25.type () == Variant::type_duration, "cast real --> duration");
    t.ok (v25.get_duration () == 3,              "cast real --> duration");

    // Variant::type_string --> *
    Variant v30 ("foo");
    v30.cast (Variant::type_boolean);
    t.ok (v30.type () == Variant::type_boolean, "cast string --> boolean");
    t.ok (v30.get_bool () == true,              "cast string --> boolean");

    Variant v31 ("42");
    v31.cast (Variant::type_integer);
    t.ok (v31.type () == Variant::type_integer, "cast string --> integer");
    t.ok (v31.get_integer () == 42,             "cast string --> integer");

    Variant v31h ("0x20");
    v31h.cast (Variant::type_integer);
    t.ok (v31h.type () == Variant::type_integer, "cast string(hex) --> integer");
    t.ok (v31h.get_integer () == 32,             "cast string(hex) --> integer");

    Variant v32 ("3.14");
    v32.cast (Variant::type_real);
    t.ok (v32.type () == Variant::type_real,    "cast string --> real");
    t.is (v32.get_real (), 3.14, EPSILON,       "cast string --> real");

    Variant v33 ("foo");
    v33.cast (Variant::type_string);
    t.ok (v33.type () == Variant::type_string,  "cast string --> string");
    t.ok (v33.get_string () == "foo",           "cast string --> string");

    Variant v34 ("2013-12-07T16:33:00-05:00");
    v34.cast (Variant::type_date);
    t.ok (v34.type () == Variant::type_date,    "cast string --> date");
    t.ok (v34.get_date () == 1386451980,        "cast string --> date");

    Variant v35 ("42 days");
    v35.cast (Variant::type_duration);
    t.ok (v35.type () == Variant::type_duration, "cast string --> duration");
    t.is (v35.get_duration (), 3628800,          "cast string --> duration");

    Variant v35b ("P42D");
    v35b.cast (Variant::type_duration);
    t.ok (v35b.type () == Variant::type_duration, "cast string --> duration");
    t.is (v35b.get_duration (), 3628800,          "cast string --> duration");

    // Variant::type_date --> *
    Variant v40 ((time_t) 1234567890, Variant::type_date);
    v40.cast (Variant::type_boolean);
    t.ok (v40.type () == Variant::type_boolean, "cast date --> boolean");
    t.ok (v40.get_bool () == true,              "cast date --> boolean");

    Variant v41 ((time_t) 1234567890, Variant::type_date);
    v41.cast (Variant::type_integer);
    t.ok (v41.type () == Variant::type_integer, "cast date --> integer");
    t.ok (v41.get_integer () == 1234567890,     "cast date --> integer");

    Variant v42 ((time_t) 1234567890, Variant::type_date);
    v42.cast (Variant::type_real);
    t.ok (v42.type () == Variant::type_real,      "cast date --> real");
    t.is (v42.get_real (), 1234567890.0, EPSILON, "cast date --> real");

    // YYYY-MM-DDThh:mm:ss
    //     ^  ^  ^  ^  ^
    Variant v43 ((time_t) 1234567890, Variant::type_date);
    v43.cast (Variant::type_string);
    t.ok (v43.type () == Variant::type_string, "cast date --> string");
    std::string s = v43.get_string ();
    t.is ((int)s[4],  (int)'-',                "cast date --> string");
    t.is ((int)s[7],  (int)'-',                "cast date --> string");
    t.is ((int)s[10], (int)'T',                "cast date --> string");
    t.is ((int)s[13], (int)':',                "cast date --> string");
    t.is ((int)s[16], (int)':',                "cast date --> string");
    t.is ((int)s.length (), 19,                "cast date --> string");

    Variant v44 ((time_t) 1234567890, Variant::type_date);
    v44.cast (Variant::type_date);
    t.ok (v44.type () == Variant::type_date,    "cast date --> date");
    t.ok (v44.get_date () == 1234567890,        "cast date --> date");

    Variant v45 ((time_t) 1234567890, Variant::type_date);
    v45.cast (Variant::type_duration);
    t.ok (v45.type () == Variant::type_duration, "cast date --> duration");
    t.ok (v45.get_duration () == 1234567890,     "cast date --> duration");

    // Variant::type_duration --> *
    Variant v50 ((time_t) 12345, Variant::type_duration);
    v50.cast (Variant::type_boolean);
    t.ok (v50.type () == Variant::type_boolean, "cast duration --> boolean");
    t.ok (v50.get_bool () == true,              "cast duration --> boolean");

    Variant v51 ((time_t) 12345, Variant::type_duration);
    v51.cast (Variant::type_integer);
    t.ok (v51.type () == Variant::type_integer, "cast duration --> integer");
    t.ok (v51.get_integer () == 12345,          "cast duration --> integer");

    Variant v52 ((time_t) 12345, Variant::type_duration);
    v52.cast (Variant::type_real);
    t.ok (v52.type () == Variant::type_real,    "cast duration --> real");
    t.is (v52.get_real (), 12345.0, EPSILON,    "cast duration --> real");

    Variant v53 ((time_t) 12345, Variant::type_duration);
    v53.cast (Variant::type_string);
    t.ok (v53.type () == Variant::type_string,  "cast duration --> string");
    t.is (v53.get_string (), "PT3H25M45S",      "cast duration --> string");

    Variant v54 ((time_t) 12345, Variant::type_duration);
    v54.cast (Variant::type_date);
    t.ok (v54.type () == Variant::type_date,    "cast duration --> date");
    t.is (v54.get_date (), 12345,               "cast duration --> date");

    Variant v55 ((time_t) 12345, Variant::type_duration);
    v55.cast (Variant::type_duration);
    t.ok (v55.type () == Variant::type_duration, "cast duration --> duration");
    t.ok (v55.get_duration () == 12345,          "cast duration --> duration");
  }

  catch (const std::string& e)
  {
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
