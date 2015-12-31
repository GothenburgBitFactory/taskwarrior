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
#include <Task.h>

Context context;
Task task;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (120);

  Variant vs0 ("untrue");         // !~ true
  Variant vs1 (8421);             // !~ 42
  Variant vs2 (3.14159);          // !~ 3.14
  Variant vs3 ("foolish");        // !~ foo

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // Interesting cases.
  Variant vs00 = vs0.operator_nomatch (v0, task);
  t.is (vs00.type (), Variant::type_boolean, "untrue !~ true --> boolean");
  t.is (vs00.get_bool (), false,             "untrue !~ true --> false");

  Variant vs01 = vs0.operator_nomatch (v1, task);
  t.is (vs01.type (), Variant::type_boolean, "untrue !~ 42 --> boolean");
  t.is (vs01.get_bool (), true,              "untrue !~ 42 --> true");

  Variant vs02 = vs0.operator_nomatch (v2, task);
  t.is (vs02.type (), Variant::type_boolean, "untrue !~ 3.14 --> boolean");
  t.is (vs02.get_bool (), true,              "untrue !~ 3.14 --> true");

  Variant vs03 = vs0.operator_nomatch (v3, task);
  t.is (vs03.type (), Variant::type_boolean, "untrue !~ 'foo' --> boolean");
  t.is (vs03.get_bool (), true,              "untrue !~ 'foo' --> true");

  Variant vs04 = vs0.operator_nomatch (v4, task);
  t.is (vs04.type (), Variant::type_boolean, "untrue !~ 1234567890 --> boolean");
  t.is (vs04.get_bool (), true,              "untrue !~ 1234567890 --> true");

  Variant vs05 = vs0.operator_nomatch (v5, task);
  t.is (vs05.type (), Variant::type_boolean, "untrue !~ 1200 --> boolean");
  t.is (vs05.get_bool (), true,              "untrue !~ 1200 --> true");

  Variant vs10 = vs1.operator_nomatch (v0, task);
  t.is (vs10.type (), Variant::type_boolean, "8421 !~ true --> boolean");
  t.is (vs10.get_bool (), true,              "8421 !~ true --> true");

  Variant vs11 = vs1.operator_nomatch (v1, task);
  t.is (vs11.type (), Variant::type_boolean, "8421 !~ 42 --> boolean");
  t.is (vs11.get_bool (), false,             "8421 !~ 42 --> false");

  Variant vs12 = vs1.operator_nomatch (v2, task);
  t.is (vs12.type (), Variant::type_boolean, "8421 !~ 3.14 --> boolean");
  t.is (vs12.get_bool (), true,              "8421 !~ 3.14 --> true");

  Variant vs13 = vs1.operator_nomatch (v3, task);
  t.is (vs13.type (), Variant::type_boolean, "8421 !~ 'foo' --> boolean");
  t.is (vs13.get_bool (), true,              "8421 !~ 'foo' --> true");

  Variant vs14 = vs1.operator_nomatch (v4, task);
  t.is (vs14.type (), Variant::type_boolean, "8421 !~ 1234567890 --> boolean");
  t.is (vs14.get_bool (), true,              "8421 !~ 1234567890 --> true");

  Variant vs15 = vs1.operator_nomatch (v5, task);
  t.is (vs15.type (), Variant::type_boolean, "8421 !~ 1200 --> boolean");
  t.is (vs15.get_bool (), true,              "8421 !~ 1200 --> true");

  Variant vs20 = vs2.operator_nomatch (v0, task);
  t.is (vs20.type (), Variant::type_boolean, "3.14159 !~ true --> boolean");
  t.is (vs20.get_bool (), true,              "3.14159 !~ true --> true");

  Variant vs21 = vs2.operator_nomatch (v1, task);
  t.is (vs21.type (), Variant::type_boolean, "3.14159 !~ 42 --> boolean");
  t.is (vs21.get_bool (), true,              "3.14159 !~ 42 --> true");

  Variant vs22 = vs2.operator_nomatch (v2, task);
  t.is (vs22.type (), Variant::type_boolean, "3.14159 !~ 3.14 --> boolean");
  t.is (vs22.get_bool (), false,             "3.14159 !~ 3.14 --> false");

  Variant vs23 = vs2.operator_nomatch (v3, task);
  t.is (vs23.type (), Variant::type_boolean, "3.14159 !~ 'foo' --> boolean");
  t.is (vs23.get_bool (), true,              "3.14159 !~ 'foo' --> true");

  Variant vs24 = vs2.operator_nomatch (v4, task);
  t.is (vs24.type (), Variant::type_boolean, "3.14159 !~ 1234567890 --> boolean");
  t.is (vs24.get_bool (), true,              "3.14159 !~ 1234567890 --> true");

  Variant vs25 = vs2.operator_nomatch (v5, task);
  t.is (vs25.type (), Variant::type_boolean, "3.14159 !~ 1200 --> boolean");
  t.is (vs25.get_bool (), true,              "3.14159 !~ 1200 --> true");

  Variant vs30 = vs3.operator_nomatch (v0, task);
  t.is (vs30.type (), Variant::type_boolean, "foolish !~ true --> boolean");
  t.is (vs30.get_bool (), true,              "foolish !~ true --> true");

  Variant vs31 = vs3.operator_nomatch (v1, task);
  t.is (vs31.type (), Variant::type_boolean, "foolish !~ 42 --> boolean");
  t.is (vs31.get_bool (), true,              "foolish !~ 42 --> true");

  Variant vs32 = vs3.operator_nomatch (v2, task);
  t.is (vs32.type (), Variant::type_boolean, "foolish !~ 3.14 --> boolean");
  t.is (vs32.get_bool (), true,              "foolish !~ 3.14 --> true");

  Variant vs33 = vs3.operator_nomatch (v3, task);
  t.is (vs33.type (), Variant::type_boolean, "foolish !~ 'foo' --> boolean");
  t.is (vs33.get_bool (), false,             "foolish !~ 'foo' --> false");

  Variant vs34 = vs3.operator_nomatch (v4, task);
  t.is (vs34.type (), Variant::type_boolean, "foolish !~ 1234567890 --> boolean");
  t.is (vs34.get_bool (), true,              "foolish !~ 1234567890 --> true");

  Variant vs35 = vs3.operator_nomatch (v5, task);
  t.is (vs35.type (), Variant::type_boolean, "foolish !~ 1200 --> boolean");
  t.is (vs35.get_bool (), true,              "foolish !~ 1200 --> true");

  // Exhaustive comparisons.
  Variant v00 = v0.operator_nomatch (v0, task);
  t.is (v00.type (), Variant::type_boolean, "true !~ true --> boolean");
  t.is (v00.get_bool (), false,             "true !~ true --> false");

  Variant v01 = v0.operator_nomatch (v1, task);
  t.is (v01.type (), Variant::type_boolean, "true !~ 42 --> boolean");
  t.is (v01.get_bool (), true,              "true !~ 42 --> true");

  Variant v02 = v0.operator_nomatch (v2, task);
  t.is (v02.type (), Variant::type_boolean, "true !~ 3.14 --> boolean");
  t.is (v02.get_bool (), true,              "true !~ 3.14 --> true");

  Variant v03 = v0.operator_nomatch (v3, task);
  t.is (v03.type (), Variant::type_boolean, "true !~ 'foo' --> boolean");
  t.is (v03.get_bool (), true,              "true !~ 'foo' --> true");

  Variant v04 = v0.operator_nomatch (v4, task);
  t.is (v04.type (), Variant::type_boolean, "true !~ 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "true !~ 1234567890 --> true");

  Variant v05 = v0.operator_nomatch (v5, task);
  t.is (v05.type (), Variant::type_boolean, "true !~ 1200 --> boolean");
  t.is (v05.get_bool (), true,              "true !~ 1200 --> true");

  Variant v10 = v1.operator_nomatch (v0, task);
  t.is (v10.type (), Variant::type_boolean, "42 !~ true --> boolean");
  t.is (v10.get_bool (), true,              "42 !~ true --> true");

  Variant v11 = v1.operator_nomatch (v1, task);
  t.is (v11.type (), Variant::type_boolean, "42 !~ 42 --> boolean");
  t.is (v11.get_bool (), false,             "42 !~ 42 --> false");

  Variant v12 = v1.operator_nomatch (v2, task);
  t.is (v12.type (), Variant::type_boolean, "42 !~ 3.14 --> boolean");
  t.is (v12.get_bool (), true,              "42 !~ 3.14 --> true");

  Variant v13 = v1.operator_nomatch (v3, task);
  t.is (v13.type (), Variant::type_boolean, "42 !~ 'foo' --> boolean");
  t.is (v13.get_bool (), true,              "42 !~ 'foo' --> true");

  Variant v14 = v1.operator_nomatch (v4, task);
  t.is (v04.type (), Variant::type_boolean, "42 !~ 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "42 !~ 1234567890 --> true");

  Variant v15 = v1.operator_nomatch (v5, task);
  t.is (v15.type (), Variant::type_boolean, "42 !~ 1200 --> boolean");
  t.is (v15.get_bool (), true,              "42 !~ 1200 --> true");

  Variant v20 = v2.operator_nomatch (v0, task);
  t.is (v20.type (), Variant::type_boolean, "3.14 !~ true --> boolean");
  t.is (v20.get_bool (), true,              "3.14 !~ true --> true");

  Variant v21 = v2.operator_nomatch (v1, task);
  t.is (v21.type (), Variant::type_boolean, "3.14 !~ 42 --> boolean");
  t.is (v21.get_bool (), true,              "3.14 !~ 42 --> true");

  Variant v22 = v2.operator_nomatch (v2, task);
  t.is (v22.type (), Variant::type_boolean, "3.14 !~ 3.14 --> boolean");
  t.is (v22.get_bool (), false,             "3.14 !~ 3.14 --> false");

  Variant v23 = v2.operator_nomatch (v3, task);
  t.is (v23.type (), Variant::type_boolean, "3.14 !~ 'foo' --> boolean");
  t.is (v23.get_bool (), true,              "3.14 !~ 'foo' --> true");

  Variant v24 = v2.operator_nomatch (v4, task);
  t.is (v24.type (), Variant::type_boolean, "3.14 !~ 1234567890 --> boolean");
  t.is (v24.get_bool (), true,              "3.14 !~ 1234567890 --> true");

  Variant v25 = v2.operator_nomatch (v5, task);
  t.is (v25.type (), Variant::type_boolean, "3.14 !~ 1200 --> boolean");
  t.is (v25.get_bool (), true,              "3.14 !~ 1200 --> true");

  Variant v30 = v3.operator_nomatch (v0, task);
  t.is (v30.type (), Variant::type_boolean, "'foo' !~ true --> boolean");
  t.is (v30.get_bool (), true,              "'foo' !~ true --> true");

  Variant v31 = v3.operator_nomatch (v1, task);
  t.is (v31.type (), Variant::type_boolean, "'foo' !~ 42 --> boolean");
  t.is (v31.get_bool (), true,              "'foo' !~ 42 --> true");

  Variant v32 = v3.operator_nomatch (v2, task);
  t.is (v32.type (), Variant::type_boolean, "'foo' !~ 3.14 --> boolean");
  t.is (v32.get_bool (), true,              "'foo' !~ 3.14 --> true");

  Variant v33 = v3.operator_nomatch (v3, task);
  t.is (v33.type (), Variant::type_boolean, "'foo' !~ 'foo' --> boolean");
  t.is (v33.get_bool (), false,             "'foo' !~ 'foo' --> false");

  Variant v34 = v3.operator_nomatch (v4, task);
  t.is (v34.type (), Variant::type_boolean, "'foo' !~ 1234567890 --> boolean");
  t.is (v34.get_bool (), true,              "'foo' !~ 1234567890 --> true");

  Variant v35 = v3.operator_nomatch (v5, task);
  t.is (v35.type (), Variant::type_boolean, "'foo' !~ 1200 --> boolean");
  t.is (v35.get_bool (), true,              "'foo' !~ 1200 --> true");

  Variant v40 = v4.operator_nomatch (v0, task);
  t.is (v40.type (), Variant::type_boolean, "1234567890 !~ true --> boolean");
  t.is (v40.get_bool (), true,              "1234567890 !~ true --> true");

  Variant v41 = v4.operator_nomatch (v1, task);
  t.is (v41.type (), Variant::type_boolean, "1234567890 !~ 42 --> boolean");
  t.is (v41.get_bool (), true,              "1234567890 !~ 42 --> true");

  Variant v42 = v4.operator_nomatch (v2, task);
  t.is (v42.type (), Variant::type_boolean, "1234567890 !~ 3.14 --> boolean");
  t.is (v42.get_bool (), true,              "1234567890 !~ 3.14 --> true");

  Variant v43 = v4.operator_nomatch (v3, task);
  t.is (v43.type (), Variant::type_boolean, "1234567890 !~ 'foo' --> boolean");
  t.is (v43.get_bool (), true,              "1234567890 !~ 'foo' --> true");

  Variant v44 = v4.operator_nomatch (v4, task);
  t.is (v44.type (), Variant::type_boolean, "1234567890 !~ 1234567890 --> boolean");
  t.is (v44.get_bool (), false,             "1234567890 !~ 1234567890 --> false");

  Variant v45 = v4.operator_nomatch (v5, task);
  t.is (v45.type (), Variant::type_boolean, "1234567890 !~ 1200 --> boolean");
  t.is (v45.get_bool (), true,              "1234567890 !~ 1200 --> true");

  Variant v50 = v5.operator_nomatch (v0, task);
  t.is (v50.type (), Variant::type_boolean, "1200 !~ true --> boolean");
  t.is (v50.get_bool (), true,              "1200 !~ true --> true");

  Variant v51 = v5.operator_nomatch (v1, task);
  t.is (v51.type (), Variant::type_boolean, "1200 !~ 42 --> boolean");
  t.is (v51.get_bool (), true,              "1200 !~ 42 --> true");

  Variant v52 = v5.operator_nomatch (v2, task);
  t.is (v52.type (), Variant::type_boolean, "1200 !~ 3.14 --> boolean");
  t.is (v52.get_bool (), true,              "1200 !~ 3.14 --> true");

  Variant v53 = v5.operator_nomatch (v3, task);
  t.is (v53.type (), Variant::type_boolean, "1200 !~ 'foo' --> boolean");
  t.is (v53.get_bool (), true,              "1200 !~ 'foo' --> true");

  Variant v54 = v5.operator_nomatch (v4, task);
  t.is (v04.type (), Variant::type_boolean, "1200 !~ 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "1200 !~ 1234567890 --> true");

  Variant v55 = v5.operator_nomatch (v5, task);
  t.is (v55.type (), Variant::type_boolean, "1200 !~ 1200 --> boolean");
  t.is (v55.get_bool (), false,             "1200 !~ 1200 --> false");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
