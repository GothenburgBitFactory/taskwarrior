////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
// cmake.h include header must come first

#include <stdlib.h>
#include <test.h>

#include "Context.h"

////////////////////////////////////////////////////////////////////////////////
int TEST_NAME(int, char**) {
  UnitTest test(48);
  Context context;
  Context::setContext(&context);

  // Ensure environment has no influence.
  unsetenv("TASKDATA");
  unsetenv("TASKRC");

  test.is((int)Task::textToStatus("pending"), (int)Task::pending, "textToStatus pending");
  test.is((int)Task::textToStatus("completed"), (int)Task::completed, "textToStatus completed");
  test.is((int)Task::textToStatus("deleted"), (int)Task::deleted, "textToStatus deleted");
  test.is((int)Task::textToStatus("recurring"), (int)Task::recurring, "textToStatus recurring");

  test.is(Task::statusToText(Task::pending), "pending", "statusToText pending");
  test.is(Task::statusToText(Task::completed), "completed", "statusToText completed");
  test.is(Task::statusToText(Task::deleted), "deleted", "statusToText deleted");
  test.is(Task::statusToText(Task::recurring), "recurring", "statusToText recurring");

  /*

  TODO Task::composeCSV
  TODO Task::composeYAML
  TODO Task::id
  TODO Task::*Status
  TODO Task::*Tag*
  TODO Task::*Annotation*

  TODO Task::addDependency
  TODO Task::addDependency
  TODO Task::removeDependency
  TODO Task::removeDependency
  TODO Task::getDependencies
  TODO Task::getDependencies

  TODO Task::urgency

  TODO Task::encode
  TODO Task::decode

  */

  // Task::operator==
  Task left("{\"one\":\"1\", \"two\":\"2\", \"three\":\"3\"}");
  Task right(left);
  test.ok(left == right, "left == right -> true");
  left.set("one", "1.0");
  test.notok(left == right, "left == right -> false");

  ////////////////////////////////////////////////////////////////////////////////
  Task task;

  // Task::set
  task = Task();
  task.set("name", "value");
  test.is(task.composeJSON(), "{\"name\":\"value\"}", "Task::set");

  // Task::has
  test.ok(task.has("name"), "Task::has");
  test.notok(task.has("woof"), "Task::has not");

  // Task::get_int
  task.set("one", 1);
  test.is(task.composeJSON(), R"({"name":"value","one":"1"})", "Task::set");
  test.is(task.get_int("one"), 1, "Task::get_int");

  // Task::get_ulong
  task.set("two", "4294967295");
  test.is(task.composeJSON(), R"({"name":"value","one":"1","two":"4294967295"})", "Task::set");
  test.is((size_t)task.get_ulong("two"), (size_t)4294967295UL, "Task::get_ulong");

  // Task::remove
  task.remove("one");
  task.remove("two");
  test.is(task.composeJSON(), "{\"name\":\"value\"}", "Task::remove");

  // Task::all
  test.is(task.all().size(), (size_t)1, "Task::all size");

  ////////////////////////////////////////////////////////////////////////////////

  Task::attributes["description"] = "string";
  Task::attributes["entry"] = "date";
  Task::attributes["tags"] = "string";
  Task::attributes["uuid"] = "string";

  bool good = true;
  try {
    Task t4("{}");
  } catch (const std::string& e) {
    test.diag(e);
    good = false;
  }
  test.ok(good, "Task::Task ('{}')");

  good = true;
  try {
    Task t5(
        R"({"uuid":"00000000-0000-0000-000000000001","description":"foo","entry":"1234567890"})");
  } catch (const std::string& e) {
    test.diag(e);
    good = false;
  }
  test.ok(good, "Task::Task ('{<minimal>}')");

  // Verify tag handling is correct
  Task t6;
  t6.set("entry", "20130602T224000Z");
  t6.set("description", "DESC");
  t6.addTag("tag1");
  test.is(t6.composeJSON(), R"({"description":"DESC","entry":"20130602T224000Z","tags":["tag1"]})",
          "JSON good");

  t6.addTag("tag2");
  test.is(t6.composeJSON(),
          R"({"description":"DESC","entry":"20130602T224000Z","tags":["tag1","tag2"]})",
          "JSON good");

  good = true;
  Task t7;
  try {
    t7 = Task(R"({"description":"DESC","entry":"20130602T224000Z","tags":["tag1","tag2"]})");
  } catch (const std::string& e) {
    test.diag(e);
    good = false;
  }
  test.ok(good, "Task::Task ('{two tags}')");
  test.is(t7.composeJSON(),
          R"({"description":"DESC","entry":"20130602T224000Z","tags":["tag1","tag2"]})",
          "JSON good");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
