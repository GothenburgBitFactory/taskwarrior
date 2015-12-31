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
#include <stdlib.h>
#include <columns/ColID.h>
#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest test (12);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  ColumnID columnID;
  unsigned int minimum = 0;
  unsigned int maximum = 0;

  Task t1;
  t1.id = 3;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 1, "id:3 --> ColID::measure minimum 1");
  test.is ((int)maximum, 1, "id:3 --> ColID::measure maximum 1");

  t1.id = 33;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 2, "id:33 --> ColID::measure minimum 2");
  test.is ((int)maximum, 2, "id:33 --> ColID::measure maximum 2");

  t1.id = 333;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 3, "id:333 --> ColID::measure minimum 3");
  test.is ((int)maximum, 3, "id:333 --> ColID::measure maximum 3");

  t1.id = 3333;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 4, "id:3333 --> ColID::measure minimum 4");
  test.is ((int)maximum, 4, "id:3333 --> ColID::measure maximum 4");

  t1.id = 33333;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 5, "id:33333 --> ColID::measure minimum 5");
  test.is ((int)maximum, 5, "id:33333 --> ColID::measure maximum 5");

  t1.id = 333333;
  columnID.measure (t1, minimum, maximum);
  test.is ((int)minimum, 6, "id:333333 --> ColID::measure minimum 6");
  test.is ((int)maximum, 6, "id:333333 --> ColID::measure maximum 6");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

