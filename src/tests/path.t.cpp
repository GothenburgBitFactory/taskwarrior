////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <Path.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (32);

  // Path ();
  Path p0;
  t.ok (p0.data == "", "Path::Path");

  // Path (const Path&);
  Path p1 = Path ("foo");
  t.ok (p1.data == "foo", "Path::operator=");

  // Path (const std::string&);
  Path p2 ("~");
  t.ok (p2.data != "~", "~ expanded to " + p2.data);

  Path p3 ("/tmp");
  t.ok (p3.data == "/tmp", "/tmp -> /tmp");

  // Path& operator= (const Path&);
  Path p3_copy (p3);
  t.is (p3.data, p3_copy.data, "Path::Path (Path&)");

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
  t.notok (p1.is_absolute (), "foo !is_absolute");
  t.ok    (p2.is_absolute (), "~ is_absolute (after expansion)");
  t.ok    (p3.is_absolute (), "/tmp is_absolute");
  t.ok    (p4.is_absolute (), "/a/b/c/file.ext is_absolute");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
