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
  UnitTest t (76);

  Variant v0 (true);
  Variant v1 (42);
  Variant v2 (3.14);
  Variant v3 ("foo");
  Variant v4 (1234567890, Variant::type_date);
  Variant v5 (1200, Variant::type_duration);

  // Truth table.
  Variant vFalse (false);
  Variant vTrue (true);
  t.is (vFalse && vFalse, false, "false && false --> false");
  t.is (vFalse && vTrue,  false, "false && true --> false");
  t.is (vTrue && vFalse,  false, "true && false --> false");
  t.is (vTrue && vTrue,   true,  "true && true --> true");

  Variant v00 = v0 && v0;
  t.is (v00.type (), Variant::type_boolean, "true && true --> boolean");
  t.is (v00.get_bool (), true,              "true && true --> true");

  Variant v01 = v0 && v1;
  t.is (v01.type (), Variant::type_boolean, "true && 42 --> boolean");
  t.is (v01.get_bool (), true,              "true && 42 --> true");

  Variant v02 = v0 && v2;
  t.is (v02.type (), Variant::type_boolean, "true && 3.14 --> boolean");
  t.is (v02.get_bool (), true,              "true && 3.14 --> true");

  Variant v03 = v0 && v3;
  t.is (v03.type (), Variant::type_boolean, "true && 'foo' --> boolean");
  t.is (v03.get_bool (), true,              "true && 'foo' --> true");

  Variant v04 = v0 && v4;
  t.is (v04.type (), Variant::type_boolean, "true && 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "true && 1234567890 --> true");

  Variant v05 = v0 && v5;
  t.is (v05.type (), Variant::type_boolean, "true && 1200 --> boolean");
  t.is (v05.get_bool (), true,              "true && 1200 --> true");

  Variant v10 = v1 && v0;
  t.is (v10.type (), Variant::type_boolean, "42 && true --> boolean");
  t.is (v10.get_bool (), true,              "42 && true --> true");

  Variant v11 = v1 && v1;
  t.is (v11.type (), Variant::type_boolean, "42 && 42 --> boolean");
  t.is (v11.get_bool (), true,              "42 && 42 --> true");

  Variant v12 = v1 && v2;
  t.is (v12.type (), Variant::type_boolean, "42 && 3.14 --> boolean");
  t.is (v12.get_bool (), true,              "42 && 3.14 --> true");

  Variant v13 = v1 && v3;
  t.is (v13.type (), Variant::type_boolean, "42 && 'foo' --> boolean");
  t.is (v13.get_bool (), true,              "42 && 'foo' --> true");

  Variant v14 = v1 && v4;
  t.is (v04.type (), Variant::type_boolean, "42 && 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "42 && 1234567890 --> true");

  Variant v15 = v1 && v5;
  t.is (v15.type (), Variant::type_boolean, "42 && 1200 --> boolean");
  t.is (v15.get_bool (), true,              "42 && 1200 --> true");

  Variant v20 = v2 && v0;
  t.is (v20.type (), Variant::type_boolean, "3.14 && true --> boolean");
  t.is (v20.get_bool (), true,              "3.14 && true --> true");

  Variant v21 = v2 && v1;
  t.is (v21.type (), Variant::type_boolean, "3.14 && 42 --> boolean");
  t.is (v21.get_bool (), true,              "3.14 && 42 --> true");

  Variant v22 = v2 && v2;
  t.is (v22.type (), Variant::type_boolean, "3.14 && 3.14 --> boolean");
  t.is (v22.get_bool (), true,              "3.14 && 3.14 --> true");

  Variant v23 = v2 && v3;
  t.is (v23.type (), Variant::type_boolean, "3.14 && 'foo' --> boolean");
  t.is (v23.get_bool (), true,              "3.14 && 'foo' --> true");

  Variant v24 = v2 && v4;
  t.is (v24.type (), Variant::type_boolean, "3.14 && 1234567890 --> boolean");
  t.is (v24.get_bool (), true,              "3.14 && 1234567890 --> true");

  Variant v25 = v2 && v5;
  t.is (v25.type (), Variant::type_boolean, "3.14 && 1200 --> boolean");
  t.is (v25.get_bool (), true,              "3.14 && 1200 --> true");

  Variant v30 = v3 && v0;
  t.is (v30.type (), Variant::type_boolean, "'foo' && true --> boolean");
  t.is (v30.get_bool (), true,              "'foo' && true --> true");

  Variant v31 = v3 && v1;
  t.is (v31.type (), Variant::type_boolean, "'foo' && 42 --> boolean");
  t.is (v31.get_bool (), true,              "'foo' && 42 --> true");

  Variant v32 = v3 && v2;
  t.is (v32.type (), Variant::type_boolean, "'foo' && 3.14 --> boolean");
  t.is (v32.get_bool (), true,              "'foo' && 3.14 --> true");

  Variant v33 = v3 && v3;
  t.is (v33.type (), Variant::type_boolean, "'foo' && 'foo' --> boolean");
  t.is (v33.get_bool (), true,              "'foo' && 'foo' --> true");

  Variant v34 = v3 && v4;
  t.is (v34.type (), Variant::type_boolean, "'foo' && 1234567890 --> boolean");
  t.is (v34.get_bool (), true,              "'foo' && 1234567890 --> true");

  Variant v35 = v3 && v5;
  t.is (v35.type (), Variant::type_boolean, "'foo' && 1200 --> boolean");
  t.is (v35.get_bool (), true,              "'foo' && 1200 --> true");

  Variant v40 = v4 && v0;
  t.is (v40.type (), Variant::type_boolean, "1234567890 && true --> boolean");
  t.is (v40.get_bool (), true,              "1234567890 && true --> true");

  Variant v41 = v4 && v1;
  t.is (v41.type (), Variant::type_boolean, "1234567890 && 42 --> boolean");
  t.is (v41.get_bool (), true,              "1234567890 && 42 --> true");

  Variant v42 = v4 && v2;
  t.is (v42.type (), Variant::type_boolean, "1234567890 && 3.14 --> boolean");
  t.is (v42.get_bool (), true,              "1234567890 && 3.14 --> true");

  Variant v43 = v4 && v3;
  t.is (v43.type (), Variant::type_boolean, "1234567890 && 'foo' --> boolean");
  t.is (v43.get_bool (), true,              "1234567890 && 'foo' --> true");

  Variant v44 = v4 && v4;
  t.is (v44.type (), Variant::type_boolean, "1234567890 && 1234567890 --> boolean");
  t.is (v44.get_bool (), true,              "1234567890 && 1234567890 --> true");

  Variant v45 = v4 && v5;
  t.is (v45.type (), Variant::type_boolean, "1234567890 && 1200 --> boolean");
  t.is (v45.get_bool (), true,              "1234567890 && 1200 --> true");

  Variant v50 = v5 && v0;
  t.is (v50.type (), Variant::type_boolean, "1200 && true --> boolean");
  t.is (v50.get_bool (), true,              "1200 && true --> true");

  Variant v51 = v5 && v1;
  t.is (v51.type (), Variant::type_boolean, "1200 && 42 --> boolean");
  t.is (v51.get_bool (), true,              "1200 && 42 --> true");

  Variant v52 = v5 && v2;
  t.is (v52.type (), Variant::type_boolean, "1200 && 3.14 --> boolean");
  t.is (v52.get_bool (), true,              "1200 && 3.14 --> true");

  Variant v53 = v5 && v3;
  t.is (v53.type (), Variant::type_boolean, "1200 && 'foo' --> boolean");
  t.is (v53.get_bool (), true,              "1200 && 'foo' --> true");

  Variant v54 = v5 && v4;
  t.is (v04.type (), Variant::type_boolean, "1200 && 1234567890 --> boolean");
  t.is (v04.get_bool (), true,              "1200 && 1234567890 --> true");

  Variant v55 = v5 && v5;
  t.is (v55.type (), Variant::type_boolean, "1200 && 1200 --> boolean");
  t.is (v55.get_bool (), true,              "1200 && 1200 --> true");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
