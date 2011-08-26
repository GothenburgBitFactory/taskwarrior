////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <algorithm>
#include <Context.h>
#include <Directory.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (25);

  // Directory (const File&);
  // Directory (const Path&);
  Directory d0 (Path ("/tmp"));
  Directory d1 (File ("/tmp"));
  Directory d2 (File (Path ("/tmp")));
  t.is (d0._data, d1._data, "Directory(std::string) == Directory (File&)");
  t.is (d0._data, d2._data, "Directory(std::string) == Directory (File (Path &))");
  t.is (d1._data, d2._data, "Directory(File&)) == Directory (File (Path &))");

  // Directory (const Directory&);
  Directory d3 (d2);
  t.is (d3._data, "/tmp", "Directory (Directory&)");

  // Directory (const std::string&);
  Directory d4 ("/tmp/test_directory");

  // Directory& operator= (const Directory&);
  Directory d5 = d4;
  t.is (d5._data, "/tmp/test_directory", "Directory::operator=");

  // operator (std::string) const;
  t.is ((std::string) d3, "/tmp", "Directory::operator (std::string) const");

  // virtual bool create ();
  t.ok (d5.create (), "Directory::create /tmp/test_directory");
  t.ok (d5.exists (), "Directory::exists /tmp/test_directory");

  Directory d6 (d5._data + "/dir");
  t.ok (d6.create (), "Directory::create /tmp/test_directory/dir");

  File::create (d5._data + "/f0");
  File::create (d6._data + "/f1");

  // std::vector <std::string> list ();
  std::vector <std::string> files = d5.list ();
  std::sort (files.begin (), files.end ());
  t.is ((int)files.size (), 2, "Directory::list 1 file");
  t.is (files[0], "/tmp/test_directory/dir", "file[0] is /tmp/test_directory/dir");
  t.is (files[1], "/tmp/test_directory/f0", "file[1] is /tmp/test_directory/f0");

  // std::vector <std::string> listRecursive ();
  files = d5.listRecursive ();
  std::sort (files.begin (), files.end ());
  t.is ((int)files.size (), 2, "Directory::list 1 file");
  t.is (files[0], "/tmp/test_directory/dir/f1", "file is /tmp/test_directory/dir/f1");
  t.is (files[1], "/tmp/test_directory/f0", "file is /tmp/test_directory/f0");

  // virtual bool remove ();
  t.ok (File::remove (d5._data + "/f0"), "File::remove /tmp/test_directory/f0");
  t.ok (File::remove (d6._data + "/f1"), "File::remove /tmp/test_directory/dir/f1");

  t.ok (d6.remove (), "Directory::remove /tmp/test_directory/dir");
  t.notok (d6.exists (), "Directory::exists /tmp/test_directory/dir - no");

  t.ok (d5.remove (), "Directory::remove /tmp/test_directory");
  t.notok (d5.exists (), "Directory::exists /tmp/test_directory - no");

  // bool remove (const std::string&);
  Directory d7 ("/tmp/to_be_removed");
  t.ok (d7.create (), "Directory::create /tmp/to_be_removed");
  File::create (d7._data + "/f0");
  Directory d8 (d7._data + "/another");
  t.ok (d8.create (), "Directory::create /tmp/to_be_removed/another");
  File::create (d8._data + "/f1");
  t.ok (d7.remove (), "Directory::remove /tmp/to_be_removed");
  t.notok (d7.exists (), "Directory /tmp/to_be_removed gone");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
