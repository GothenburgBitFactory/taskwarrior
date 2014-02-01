////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <Context.h>
#include <Path.h>
#include <Directory.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (32);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  // Path ();
  Path p0;
  t.is (p0._data, "", "Path::Path");

  // Path (const Path&);
  Path p1 = Path ("foo");
  t.is (p1._data, Directory::cwd () + "/foo", "Path::operator=");

  // Path (const std::string&);
  Path p2 ("~");
  t.ok (p2._data != "~", "~ expanded to " + p2._data);

  Path p3 ("/tmp");
  t.ok (p3._data == "/tmp", "/tmp -> /tmp");

  // Path& operator= (const Path&);
  Path p3_copy (p3);
  t.is (p3._data, p3_copy._data, "Path::Path (Path&)");

  // operator (std::string) const;
  t.is ((std::string) p3, "/tmp", "Path::operator (std::string) const");

  // std::string name () const;
  Path p4 ("/a/b/c/file.ext");
  t.is (p4.name (), "file.ext",  "/a/b/c/file.ext name is file.ext");

  // std::string parent () const;
  t.is (p4.parent (), "/a/b/c",  "/a/b/c/file.ext parent is /a/b/c");

  // std::string extension () const;
  t.is (p4.extension (), "ext",  "/a/b/c/file.ext extension is ext");

  // bool exists () const;
  t.ok (p2.exists (), "~ exists");
  t.ok (p3.exists (), "/tmp exists");

  // bool is_directory () const;
  t.ok (p2.is_directory (), "~ is_directory");
  t.ok (p3.is_directory (), "/tmp is_directory");

  // bool readable () const;
  t.ok (p2.readable (), "~ readable");
  t.ok (p3.readable (), "/tmp readable");

  // bool writable () const;
  t.ok (p2.writable (), "~ writable");
  t.ok (p3.writable (), "/tmp writable");

  // bool executable () const;
  t.ok (p2.executable (), "~ executable");
  t.ok (p3.executable (), "/tmp executable");

  // static std::string expand (const std::string&);
  t.ok (Path::expand ("~") != "~", "Path::expand ~ != ~");
  t.ok (Path::expand ("~/") != "~/", "Path::expand ~/ != ~/");

  // static std::vector <std::string> glob (const std::string&);
  std::vector <std::string> out = Path::glob ("/tmp");
  t.ok (out.size () == 1, "/tmp -> 1 result");
  t.is (out[0], "/tmp", "/tmp -> /tmp");

  out = Path::glob ("/t?p");
  t.ok (out.size () == 1, "/t?p -> 1 result");
  t.is (out[0], "/tmp", "/t?p -> /tmp");

  out = Path::glob ("/[s-u]mp");
  t.ok (out.size () == 1, "/[s-u]mp -> 1 result");
  t.is (out[0], "/tmp", "/[s-u]mp -> /tmp");

  // bool is_absolute () const;
  t.notok (p0.is_absolute (), "'' !is_absolute");
  t.ok    (p1.is_absolute (), "foo is_absolute");
  t.ok    (p2.is_absolute (), "~ is_absolute (after expansion)");
  t.ok    (p3.is_absolute (), "/tmp is_absolute");
  t.ok    (p4.is_absolute (), "/a/b/c/file.ext is_absolute");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
