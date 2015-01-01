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

#include <cmake.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <DOM.h>
#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (17);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  try
  {
    // Prime the pump.
    const char* fake_argv[] = {"task"};
    context.cli.initialize (1, fake_argv);
    context.config.set ("name", "value");

    DOM dom;
    Variant result;
    t.ok (dom.get ("system.version", result),  "DOM system.version -> true");
    t.is ((std::string) result, VERSION,       "DOM system.version -> VERSION");

    t.ok (dom.get ("system.os", result),       "DOM system.os -> true");
    t.ok ((std::string) result != "<unknown>", "DOM system.os -> != Unknown");

    t.ok (dom.get ("context.program", result), "DOM context.program -> true");
    t.is ((std::string) result, "task",        "DOM context.program -> 'task'");

    t.ok (dom.get ("context.args", result),    "DOM context.args -> true");
    t.is ((std::string) result, "task",        "DOM context.args -> 'task'");

    t.ok (dom.get ("context.width", result),   "DOM context.width -> true");
    t.ok (result.get_integer () != 0,          "DOM context.width -> '0'");

    t.ok (dom.get ("context.height", result),  "DOM context.height -> true");
    t.ok (result.get_integer () != 0,          "DOM context.height -> '0'");

    // dom.get rc.name
    t.ok (dom.get ("rc.name", result),         "DOM rc.name -> true");
    t.is ((std::string) result, "value",       "DOM rc.name -> value");

    // dom.get rc.missing
    t.notok (dom.get ("rc.missing", result),   "DOM rc.missing -> false");

    // dom.set rc.name
    dom.set ("rc.new", Variant ("value"));
    t.ok (dom.get ("rc.new", result),          "DOM rc.new -> true");
    t.is ((std::string) result, "value",       "DOM rc.new -> value");
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

