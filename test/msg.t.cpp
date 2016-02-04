////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <Msg.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (13);

  Msg m;
  t.is (m.serialize (), "\n\n",        "Msg::serialize '' --> '\\n\\n'");

  m.set ("name", "value");
  t.is (m.serialize (), "name: value\n\n\n",
                                       "Msg::serialize 1 var");

  m.set ("foo", "bar");
  t.is (m.serialize (), "foo: bar\nname: value\n\n\n",
                                       "Msg::serialize 2 vars");

  m.setPayload ("payload");
  t.is (m.serialize (), "foo: bar\nname: value\n\npayload\n",
                                       "Msg::serialize 2 vars + payload");

  Msg m2;
  t.ok (m2.parse ("foo: bar\nname: value\n\npayload\n"),
                                       "Msg::parse ok");
  t.is (m2.get ("foo"),   "bar",       "Msg::get");
  t.is (m2.get ("name"),  "value",     "Msg::get");
  t.is (m2.getPayload (), "payload\n", "Msg::getPayload");

  Msg m3;
  t.ok (m3.parse ("foo:bar\nname:   value\n\npayload\n"),
                                       "Msg::parse ok");
  t.is (m3.get ("foo"),   "bar",       "Msg::get");
  t.is (m3.get ("name"),  "value",     "Msg::get");
  t.is (m3.getPayload (), "payload\n", "Msg::getPayload");
  t.ok (m3.all () == std::vector <std::string> {"foo", "name"},
                                       "Msg::all --> {'foo', 'name'}");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
