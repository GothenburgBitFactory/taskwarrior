////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
#include <string.h>
#include <math.h>
#include <test.h>

///////////////////////////////////////////////////////////////////////////////
UnitTest::UnitTest ()
: mPlanned (0)
, mCounter (0)
, mPassed (0)
, mFailed (0)
, mSkipped (0)
{
}

///////////////////////////////////////////////////////////////////////////////
UnitTest::UnitTest (int planned)
: mPlanned (planned)
, mCounter (0)
, mPassed (0)
, mFailed (0)
, mSkipped (0)
{
  std::cout << "1.." << mPlanned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
UnitTest::~UnitTest ()
{
  float percentPassed = 0.0;
  if (mPlanned > 0)
    percentPassed = (100.0 * mPassed) / std::max (mPlanned, mPassed + mFailed + mSkipped);

  if (mCounter < mPlanned)
  {
    std::cout << "# Only "
              << mCounter
              << " tests, out of a planned "
              << mPlanned
              << " were run.\n";
    mSkipped += mPlanned - mCounter;
  }

  else if (mCounter > mPlanned)
    std::cout << "# "
              << mCounter
              << " tests were run, but only "
              << mPlanned
              << " were planned.\n";

  std::cout << "# "
            << mPassed
            << " passed, "
            << mFailed
            << " failed, "
            << mSkipped
            << " skipped. "
            << std::setprecision (3) << percentPassed
            << "% passed.\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::plan (int planned)
{
  mPlanned = planned;
  mCounter = 0;
  mPassed = 0;
  mFailed = 0;
  mSkipped = 0;

  std::cout << "1.." << mPlanned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::planMore (int extra)
{
  mPlanned += extra;
  std::cout << "1.." << mPlanned << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::ok (bool expression, const std::string& name)
{
  ++mCounter;

  if (expression)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::notok (bool expression, const std::string& name)
{
  ++mCounter;

  if (!expression)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::is (bool actual, bool expected, const std::string& name)
{
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (fabs (actual - expected) <= tolerance)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (actual == expected)
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  ++mCounter;
  if (! strcmp (actual, expected))
  {
    ++mPassed;
    std::cout << "ok "
              << mCounter
              << " - "
              << name
              << "\n";
  }
  else
  {
    ++mFailed;
    std::cout << "not ok "
              << mCounter
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
  std::string::size_type start = text.find_first_not_of (" \t\n\r\f");
  std::string::size_type end   = text.find_last_not_of  (" \t\n\r\f");
  std::cout << "# " << text.substr (start, end - start + 1) << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::pass (const std::string& text)
{
  ++mCounter;
  ++mPassed;
  std::cout << "ok "
            << mCounter
            << " "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::fail (const std::string& text)
{
  ++mCounter;
  ++mFailed;
  std::cout << "not ok "
            << mCounter
            << " "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
void UnitTest::skip (const std::string& text)
{
  ++mCounter;
  ++mSkipped;
  std::cout << "skip "
            << mCounter
            << " "
            << text
            << "\n";
}

///////////////////////////////////////////////////////////////////////////////
