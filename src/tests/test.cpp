////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include <string>
#include <task.h>

static int total = 0;
static int counter = 0;

///////////////////////////////////////////////////////////////////////////////
static void check (void)
{
  if (counter > total)
    std::cout << "# Warning: There are more tests than planned."
              << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void plan (int quantity)
{
  total = quantity;
  std::cout << "1.." << quantity << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void ok (bool expression, const std::string& name)
{
  ++counter;

  if (expression)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void notok (bool expression, const std::string& name)
{
  ++counter;

  if (!expression)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (bool actual, bool expected, const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: "
              << expected
              << std::endl
              << "#      got: "
              << actual
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (size_t actual, size_t expected, const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: "
              << expected
              << std::endl
              << "#      got: "
              << actual
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (int actual, int expected, const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: "
              << expected
              << std::endl
              << "#      got: "
              << actual
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (double actual, double expected, const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: "
              << expected
              << std::endl
              << "#      got: "
              << actual
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (char actual, char expected, const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: "
              << expected
              << std::endl
              << "#      got: "
              << actual
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void is (
  const std::string& actual,
  const std::string& expected,
  const std::string& name)
{
  ++counter;
  if (actual == expected)
    std::cout << "ok "
              << counter
              << " - "
              << name
              << std::endl;
  else
    std::cout << "not ok "
              << counter
              << " - "
              << name
              << std::endl
              << "# expected: '"
              << expected
              << "'"
              << std::endl
              << "#      got: '"
              << actual
              << "'"
              << std::endl;
  check ();
}

///////////////////////////////////////////////////////////////////////////////
void diag (const std::string& text)
{
  std::string trimmed = trim (text, " \t\n\r\f");

  std::cout << "# " << trimmed << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void pass (const std::string& text)
{
  ++counter;
  std::cout << "ok "
            << counter
            << " "
            << text
            << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void fail (const std::string& text)
{
  ++counter;
  std::cout << "not ok "
            << counter
            << " "
            << text
            << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
