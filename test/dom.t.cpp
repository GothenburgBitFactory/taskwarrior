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
#include <unistd.h>
#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (6);

  try
  {
    // Prime the pump.
    context.a3.capture ("task");

    // TODO dom.get rc.name
    DOM dom;
    t.is (dom.get ("system.version"),     VERSION,     "DOM system.version -> VERSION");
    t.ok (dom.get ("system.os") != "<unknown>",        "DOM system.os -> != Unknown");
    t.is (dom.get ("context.program"),    "task",      "DOM context.program -> 'task'");
    t.is (dom.get ("context.args"),       "task",      "DOM context.args -> 'task'");
    t.is (dom.get ("context.width"),      "0",         "DOM context.width -> '0'");
    t.is (dom.get ("context.height"),     "0",         "DOM context.height -> '0'");

    // TODO dom.set rc.name
  }

  catch (const std::string& error)
  {
    t.diag (error);
    return -1;
  }

  catch (...)
  {
    t.diag ("Unknown error.");
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

