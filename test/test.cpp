////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <main.h>
#include <util.h>
#include <text.h>
#include "test.h"

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
    percentPassed = (100.0 * mPassed) / max (mPlanned, mPassed + mFailed + mSkipped);

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
  std::string trimmed = trim (text, " \t\n\r\f");

  std::cout << "# " << trimmed << "\n";
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
