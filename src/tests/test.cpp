////////////////////////////////////////////////////////////////////////////////
// Copyright 2007, 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <library.h>

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
