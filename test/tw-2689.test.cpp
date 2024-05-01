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

#include <iostream>
#include <cmake.h>
#include <stdlib.h>
#include <main.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest test (12);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  // Inform Task about the attributes in the JSON below
  Task::attributes["depends"] = "string";
  Task::attributes["uuid"] = "string";

  // depends in [..] string from a taskserver (issue#2689)
  auto sample = "{"
           "\"depends\":\"[\\\"92a40a34-37f3-4785-8ca1-ff89cfbfd105\\\",\\\"e08e35fa-e42b-4de0-acc4-518fca8f6365\\\"]\","
           "\"uuid\":\"00000000-0000-0000-0000-000000000000\""
           "}";
  auto json = Task (sample);
  auto value = json.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "json [..] uuid");
  value = json.get ("depends");
  test.is (value, "92a40a34-37f3-4785-8ca1-ff89cfbfd105,e08e35fa-e42b-4de0-acc4-518fca8f6365", "json [..] depends");
  test.ok (json.has ("dep_92a40a34-37f3-4785-8ca1-ff89cfbfd105"), "json [..] dep attr");
  test.ok (json.has ("dep_e08e35fa-e42b-4de0-acc4-518fca8f6365"), "json [..] dep attr");

  // depends in comma-delimited string from a taskserver (deprecated format)
  sample = "{"
           "\"depends\":\"92a40a34-37f3-4785-8ca1-ff89cfbfd105,e08e35fa-e42b-4de0-acc4-518fca8f6365\","
           "\"uuid\":\"00000000-0000-0000-0000-000000000000\""
           "}";
  json = Task (sample);
  value = json.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "json comma-separated uuid");
  value = json.get ("depends");
  test.is (value, "92a40a34-37f3-4785-8ca1-ff89cfbfd105,e08e35fa-e42b-4de0-acc4-518fca8f6365", "json comma-separated depends");
  test.ok (json.has ("dep_92a40a34-37f3-4785-8ca1-ff89cfbfd105"), "json comma-separated dep attr");
  test.ok (json.has ("dep_e08e35fa-e42b-4de0-acc4-518fca8f6365"), "json comma-separated dep attr");

  // depends in a JSON array from a taskserver
  sample = "{"
           "\"depends\":[\"92a40a34-37f3-4785-8ca1-ff89cfbfd105\",\"e08e35fa-e42b-4de0-acc4-518fca8f6365\"],"
           "\"uuid\":\"00000000-0000-0000-0000-000000000000\""
           "}";
  json = Task (sample);
  value = json.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "json array uuid");
  value = json.get ("depends");
  test.is (value, "92a40a34-37f3-4785-8ca1-ff89cfbfd105,e08e35fa-e42b-4de0-acc4-518fca8f6365", "json array depends");
  test.ok (json.has ("dep_92a40a34-37f3-4785-8ca1-ff89cfbfd105"), "json array dep attr");
  test.ok (json.has ("dep_e08e35fa-e42b-4de0-acc4-518fca8f6365"), "json array dep attr");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

