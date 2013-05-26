////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <text.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (16);

  t.is (mk_wcwidth ('a'),    1, "U+0061 --> 1");

  t.is (mk_wcwidth (0x5149), 2, "U+5149 --> 2");
  t.is (mk_wcwidth (0x9a8c), 2, "U+9a8c --> 2");
  t.is (mk_wcwidth (0x4e70), 2, "U+4e70 --> 2");
  t.is (mk_wcwidth (0x94b1), 2, "U+94b1 --> 2");
  t.is (mk_wcwidth (0x5305), 2, "U+5305 --> 2");
  t.is (mk_wcwidth (0x91cd), 2, "U+91cd --> 2");
  t.is (mk_wcwidth (0x65b0), 2, "U+65b0 --> 2");
  t.is (mk_wcwidth (0x8bbe), 2, "U+8bbe --> 2");
  t.is (mk_wcwidth (0x8ba1), 2, "U+8ba1 --> 2");
  t.is (mk_wcwidth (0x5411), 2, "U+5411 --> 2");
  t.is (mk_wcwidth (0x4e0a), 2, "U+4e0a --> 2");
  t.is (mk_wcwidth (0x4e0b), 2, "U+4e0b --> 2");
  t.is (mk_wcwidth (0x7bad), 2, "U+7bad --> 2");
  t.is (mk_wcwidth (0x5934), 2, "U+5934 --> 2");
  t.is (mk_wcwidth (0xff0c), 2, "U+ff0c --> 2"); // comma

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
