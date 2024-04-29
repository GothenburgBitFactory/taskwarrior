////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2021, Göteborg Bit Factory.
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
#include <test.h>
#include <DOM.h>
#include <Variant.h>

////////////////////////////////////////////////////////////////////////////////
bool providerString (const std::string& path, Variant& var)
{
  if (path == "name")
  {
    var = Variant ("value");
    return true;
  }
  else if (path == "name.next")
  {
    var = Variant ("value.next");
    return true;
  }
  else if (path == "foo")
  {
    var = Variant ("bar");
    return true;
  }
  else if (path == "name.size")
  {
    var = Variant (6);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (12);

  DOM dom;
  t.is (dom.count (), 0,               "DOM empty count is zero");

  dom.addSource ("name",      &providerString);
  dom.addSource ("name.next", &providerString);
  dom.addSource ("name.size", &providerString);
  dom.addSource ("foo",       &providerString);
  t.diag (dom.dump ());
  t.is (dom.count (), 4,               "DOM now contains 4 nodes");

  t.ok    (dom.valid ("name"),         "DOM 'name' valid");
  t.ok    (dom.valid ("name.next"),    "DOM 'name.next' valid");
  t.ok    (dom.valid ("name.size"),    "DOM 'name.size' valid");
  t.ok    (dom.valid ("foo"),          "DOM 'foo' valid");
  t.notok (dom.valid ("missing"),      "DOM 'missing' not valid");

  auto v = dom.get ("name");
  t.is (v.get_string (), "value",      "DOM get 'name' --> 'value'");

  v = dom.get ("name.next");
  t.is (v.get_string (), "value.next", "DOM get 'name.next' --> 'value.next'");

  v = dom.get ("name.size");
  t.is (v.get_integer (), 6,           "DOM get 'name.size' --> 6");

  v = dom.get ("foo");
  t.is (v.get_string (), "bar",        "DOM get 'name.size' --> 6");

  v = dom.get ("missing");
  t.is (v.get_string (), "",           "DOM get 'missing' --> ''");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
