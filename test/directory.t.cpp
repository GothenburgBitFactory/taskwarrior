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
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <Directory.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (49);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  Directory tmp ("tmp");
  tmp.create ();
  t.ok (tmp.exists (), "tmp dir created.");

  // Directory (const File&);
  // Directory (const Path&);
  Directory d0 (Path ("tmp"));
  Directory d1 (File ("tmp"));
  Directory d2 (File (Path ("tmp")));
  t.is (d0._data, d1._data, "Directory(std::string) == Directory (File&)");
  t.is (d0._data, d2._data, "Directory(std::string) == Directory (File (Path &))");
  t.is (d1._data, d2._data, "Directory(File&)) == Directory (File (Path &))");

  // Directory (const Directory&);
  Directory d3 (d2);
  t.is (d3._data, Directory::cwd () + "/tmp", "Directory (Directory&)");

  // Directory (const std::string&);
  Directory d4 ("tmp/test_directory");

  // Directory& operator= (const Directory&);
  Directory d5 = d4;
  t.is (d5._data, Directory::cwd () + "/tmp/test_directory", "Directory::operator=");

  // operator (std::string) const;
  t.is ((std::string) d3, Directory::cwd () + "/tmp", "Directory::operator (std::string) const");

  // virtual bool create ();
  t.ok (d5.create (), "Directory::create tmp/test_directory");
  t.ok (d5.exists (), "Directory::exists tmp/test_directory");

  Directory d6 (d5._data + "/dir");
  t.ok (d6.create (), "Directory::create tmp/test_directory/dir");

  File::create (d5._data + "/f0");
  File::create (d6._data + "/f1");

  // std::vector <std::string> list ();
  std::vector <std::string> files = d5.list ();
  std::sort (files.begin (), files.end ());
  t.is ((int)files.size (), 2, "Directory::list 1 file");
  t.is (files[0], Directory::cwd () + "/tmp/test_directory/dir", "file[0] is tmp/test_directory/dir");
  t.is (files[1], Directory::cwd () + "/tmp/test_directory/f0", "file[1] is tmp/test_directory/f0");

  // std::vector <std::string> listRecursive ();
  files = d5.listRecursive ();
  std::sort (files.begin (), files.end ());
  t.is ((int)files.size (), 2, "Directory::list 1 file");
  t.is (files[0], Directory::cwd () + "/tmp/test_directory/dir/f1", "file is tmp/test_directory/dir/f1");
  t.is (files[1], Directory::cwd () + "/tmp/test_directory/f0", "file is tmp/test_directory/f0");

  // virtual bool remove ();
  t.ok (File::remove (d5._data + "/f0"), "File::remove tmp/test_directory/f0");
  t.ok (File::remove (d6._data + "/f1"), "File::remove tmp/test_directory/dir/f1");

  t.ok (d6.remove (), "Directory::remove tmp/test_directory/dir");
  t.notok (d6.exists (), "Directory::exists tmp/test_directory/dir - no");

  t.ok (d5.remove (), "Directory::remove tmp/test_directory");
  t.notok (d5.exists (), "Directory::exists tmp/test_directory - no");

  // bool remove (const std::string&);
  Directory d7 ("tmp/to_be_removed");
  t.ok (d7.create (), "Directory::create tmp/to_be_removed");
  File::create (d7._data + "/f0");
  Directory d8 (d7._data + "/another");
  t.ok (d8.create (), "Directory::create tmp/to_be_removed/another");
  File::create (d8._data + "/f1");
  t.ok (d7.remove (), "Directory::remove tmp/to_be_removed");
  t.notok (d7.exists (), "Directory tmp/to_be_removed gone");

  // static std::string cwd ();
  std::string cwd = Directory::cwd ();
  t.ok (cwd.length () > 0, "Directory::cwd returned a value");

  // bool parent (std::string&) const;
  Directory d9 ("/one/two/three/four.txt");
  t.ok (d9.up (),                   "parent /one/two/three/four.txt --> true");
  t.is (d9._data, "/one/two/three", "parent /one/two/three/four.txt --> /one/two/three");
  t.ok (d9.up (),                   "parent /one/two/three --> true");
  t.is (d9._data, "/one/two",       "parent /one/two/three --> /one/two");
  t.ok (d9.up (),                   "parent /one/two --> true");
  t.is (d9._data, "/one",           "parent /one/two --> /one");
  t.ok (d9.up (),                   "parent /one --> true");
  t.is (d9._data, "/",              "parent /one --> /");
  t.notok (d9.up (),                "parent / --> false");

  // Test permissions.
  umask(0022);
  Directory d10 ("tmp/dir.perm");
  d10.create (0750);
  t.ok (d10.exists (),               "Directory::create perm file exists");
  mode_t m = d10.mode ();
  t.ok    (m & S_IFDIR,             "Directory::mode tmp/dir.perm S_IFDIR good");
  t.ok    (m & S_IRUSR,             "Directory::mode tmp/dir.perm r-------- good");
  t.ok    (m & S_IWUSR,             "Directory::mode tmp/dir.perm -w------- good");
  t.ok    (m & S_IXUSR,             "Directory::mode tmp/dir.perm --x------ good");
  t.ok    (m & S_IRGRP,             "Directory::mode tmp/dir.perm ---r----- good");
  t.notok (m & S_IWGRP,             "Directory::mode tmp/dir.perm ----w---- good");
  t.ok    (m & S_IXGRP,             "Directory::mode tmp/dir.perm -----x--- good");
  t.notok (m & S_IROTH,             "Directory::mode tmp/dir.perm ------r-- good");
  t.notok (m & S_IWOTH,             "Directory::mode tmp/dir.perm -------w- good");
  t.notok (m & S_IXOTH,             "Directory::mode tmp/dir.perm --------x good");
  d10.remove ();
  t.notok (d10.exists (),           "Directory::remove temp/dir.perm file no longer exists");

  tmp.remove ();
  t.notok (tmp.exists (),           "tmp dir removed.");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
