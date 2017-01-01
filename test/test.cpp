////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <test.h>

///////////////////////////////////////////////////////////////////////////////
UnitTest::UnitTest ()
: _planned (0)
, _counter (0)
, _passed (0)
, _failed (0)
, _skipped (0)
{
}

///////////////////////////////////////////////////////////////////////////////
UnitTest::UnitTest (int planned)
: _planned (planned)
, _counter (0)
, _passed (0)
, _failed (0)
, _skipped (0)
{
  std::cout << "1.." << _planned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
UnitTest::~UnitTest ()
{
  float percentPassed = 0.0;
  if (_planned > 0)
    percentPassed = (100.0 * _passed) / std::max (_planned, _passed + _failed + _skipped);

  if (_counter < _planned)
  {
    std::cout << "# Only "
              << _counter
              << " tests, out of a planned "
              << _planned
              << " were run.\n";
    _skipped += _planned - _counter;
  }

  else if (_counter > _planned)
    std::cout << "# "
              << _counter
              << " tests were run, but only "
              << _planned
              << " were planned.\n";

  std::cout << "# "
            << _passed
            << " passed, "
            << _failed
            << " failed, "
            << _skipped
            << " skipped. "
            << std::setprecision (3) << percentPassed
            << "% passed.\n";
  exit (_failed > 0);
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::plan (int planned)
{
  _planned = planned;
  _counter = 0;
  _passed = 0;
  _failed = 0;
  _skipped = 0;

  std::cout << "1.." << _planned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::planMore (int extra)
{
  _planned += extra;
  std::cout << "1.." << _planned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::ok (bool expression, const std::string& name)
{
  ++_counter;

  if (expression)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::notok (bool expression, const std::string& name)
{
  ++_counter;

  if (!expression)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (bool actual, bool expected, const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (size_t actual, size_t expected, const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (int actual, int expected, const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (double actual, double expected, const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (double actual, double expected, double tolerance, const std::string& name)
{
  ++_counter;
  if (fabs (actual - expected) <= tolerance)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (unsigned char actual, unsigned char expected, const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: "
              << expected
              << "\n#      got: "
              << actual
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (
  const std::string& actual,
  const std::string& expected,
  const std::string& name)
{
  ++_counter;
  if (actual == expected)
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: '"
              << expected
              << "'"
              << "\n#      got: '"
              << actual
              << "'\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (
  const char* actual,
  const char* expected,
  const std::string& name)
{
  ++_counter;
  if (! strcmp (actual, expected))
  {
    ++_passed;
    std::cout << green ("ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++_failed;
    std::cout << red ("not ok")
              << " "
              << _counter
              << " - "
              << name
              << "\n# expected: '"
              << expected
              << "'"
              << "\n#      got: '"
              << actual
              << "'\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::diag (const std::string& text)
{
  auto start = text.find_first_not_of (" \t\n\r\f");
  auto end   = text.find_last_not_of  (" \t\n\r\f");
  if (start != std::string::npos &&
      end   != std::string::npos)
    std::cout << "# " << text.substr (start, end - start + 1) << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::pass (const std::string& text)
{
  ++_counter;
  ++_passed;
  std::cout << green ("ok")
            << " "
            << _counter
            << " - "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::fail (const std::string& text)
{
  ++_counter;
  ++_failed;
  std::cout << red ("not ok")
            << " "
            << _counter
            << " - "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::skip (const std::string& text)
{
  ++_counter;
  ++_skipped;
  std::cout << yellow ("skip")
            << " "
            << _counter
            << " - "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
std::string UnitTest::red (const std::string& input)
{
  if (isatty (fileno (stdout)))
    return std::string ("\033[31m" + input + "\033[0m");

  return input;
}

///////////////////////////////////////////////////////////////////////////////
std::string UnitTest::green (const std::string& input)
{
  if (isatty (fileno (stdout)))
    return std::string ("\033[32m" + input + "\033[0m");

  return input;
}

///////////////////////////////////////////////////////////////////////////////
std::string UnitTest::yellow (const std::string& input)
{
  if (isatty (fileno (stdout)))
    return std::string ("\033[33m" + input + "\033[0m");

  return input;
}

///////////////////////////////////////////////////////////////////////////////
