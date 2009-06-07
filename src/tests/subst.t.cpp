////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <Context.h>
#include <T2.h>
#include <Subst.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (15);

  T2 task;
  task.set ("description", "one two three four");

  Subst s;
  t.ok (s.valid ("/a/b/x"),         "valid /a/b/x");
  t.ok (s.valid ("/a/b/"),          "valid /a/b/");
  t.ok (s.valid ("/two/TWO/"),      "valid /two/TWO/");
  t.ok (s.valid ("/e /E /g"),       "valid /e /E /g");
  t.ok (s.valid ("/from/to/g"),     "valid /from/to/g");
  t.ok (s.valid ("/long string//"), "valid /long string//");
  t.ok (s.valid ("//fail/"),        "valid //fail/");

  bool good = true;
  try { s.parse ("/a/b/x"); } catch (...) { good = false; }
  t.notok (good, "failed /a/b/x");

  good = true;
  try { s.parse ("//to/"); } catch (...) { good = false; }
  t.notok (good, "failed //to/");

  good = true;
  try { s.parse ("/two/TWO/"); } catch (...) { good = false; }
  t.ok (good, "parsed /two/TWO/");
  if (good)
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "one TWO three four", "single word subst");
  }
  else
  {
    t.fail ("failed to parse '/two/TWO/'");
  }

  good = true;
  try { s.parse ("/e /E /g"); } catch (...) { good = false; }
  t.ok (good, "parsed /e /E /g");
  if (good)
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "onE two threE four", "multiple word subst");
  }
  else
  {
    t.fail ("failed to parse '/e /E /g'");
  }

  good = true;
  try { s.parse ("/from/to/g"); } catch (...) { good = false; }
  t.ok (good, "parsed /from/to/g");
  if (good)
  {
    std::string description = task.get ("description");
    std::vector <Att> annotations;
    task.getAnnotations (annotations);

    s.apply (description, annotations);
    t.is (description, "one two three four", "multiple word subst mismatch");
  }
  else
  {
    t.fail ("failed to parse '/from/to/g'");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
