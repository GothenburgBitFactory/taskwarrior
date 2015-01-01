////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_UNITTEST
#define INCLUDED_UNITTEST

#include <string>

class UnitTest
{
public:
  UnitTest ();
  UnitTest (int);
  ~UnitTest ();

  void plan (int);
  void planMore (int);
  void ok (bool, const std::string&);
  void notok (bool, const std::string&);
  void is (bool, bool, const std::string&);
  void is (size_t, size_t, const std::string&);
  void is (int, int, const std::string&);
  void is (double, double, const std::string&);
  void is (double, double, double, const std::string&);
  void is (unsigned char, unsigned char, const std::string&);
  void is (const std::string&, const std::string&, const std::string&);
  void is (const char*, const char*, const std::string&);
  void diag (const std::string&);
  void pass (const std::string&);
  void fail (const std::string&);
  void skip (const std::string&);

private:
  int _planned;
  int _counter;
  int _passed;
  int _failed;
  int _skipped;
};

#endif

////////////////////////////////////////////////////////////////////////////////
