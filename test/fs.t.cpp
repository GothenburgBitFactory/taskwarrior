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
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <Context.h>
#include <FS.h>
#include <test.h>

Context context;

int main (int, char**)
{
  UnitTest t (116);

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

  // operator==
  t.notok (p2 == p3, "p2 != p3");

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

  // bool is_link () const;
  t.notok (p2.is_link (), "~ !is_link");

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

  Path p5 ("~/file.ext");
  t.notok (p5.name () == "~/file.ext", "~/file.ext --> ! ~/file.ext");

  Directory tmp ("tmp");
  tmp.create ();
  t.ok (tmp.exists (), "tmp dir created.");

  File::write ("tmp/file.t.txt", "This is a test\n");
  File f6 ("tmp/file.t.txt");
  t.ok (f6.size () == 15, "File::size tmp/file.t.txt good");
  t.ok (f6.mode () & S_IRUSR, "File::mode tmp/file.t.txt good");
  t.ok (File::remove ("tmp/file.t.txt"), "File::remove tmp/file.t.txt good");

  // operator (std::string) const;
  t.is ((std::string) f6, Directory::cwd () + "/tmp/file.t.txt", "File::operator (std::string) const");

  t.ok (File::create ("tmp/file.t.create"), "File::create tmp/file.t.create good");
  t.ok (File::remove ("tmp/file.t.create"), "File::remove tmp/file.t.create good");

  // basename (std::string) const;
  t.is (f6.name (), "file.t.txt", "File::basename tmp/file.t.txt --> file.t.txt");

  // dirname (std::string) const;
  t.is (f6.parent (), Directory::cwd () + "/tmp", "File::dirname tmp/file.t.txt --> tmp");

  // bool rename (const std::string&);
  File f7 ("tmp/file.t.2.txt");
  f7.append ("something\n");
  f7.close ();

  t.ok (f7.rename ("tmp/file.t.3.txt"),  "File::rename did not fail");
  t.is (f7._data, Directory::cwd () + "/tmp/file.t.3.txt",    "File::rename stored new name");
  t.ok (f7.exists (),                    "File::rename new file exists");
  t.ok (f7.remove (),                    "File::remove tmp/file.t.3.txt good");
  t.notok (f7.exists (),                 "File::remove new file no longer exists");

  // Test permissions.
  File f8 ("tmp/file.t.perm.txt");
  f8.create (0744);
  t.ok (f8.exists (),                    "File::create perm file exists");
  mode_t m = f8.mode ();
  t.ok    (m & S_IFREG,                  "File::mode tmp/file.t.perm.txt S_IFREG good");
  t.ok    (m & S_IRUSR,                  "File::mode tmp/file.t.perm.txt r-------- good");
  t.ok    (m & S_IWUSR,                  "File::mode tmp/file.t.perm.txt -w------- good");
  t.ok    (m & S_IXUSR,                  "File::mode tmp/file.t.perm.txt --x------ good");
  t.ok    (m & S_IRGRP,                  "File::mode tmp/file.t.perm.txt ---r----- good");
  t.notok (m & S_IWGRP,                  "File::mode tmp/file.t.perm.txt ----w---- good");
  t.notok (m & S_IXGRP,                  "File::mode tmp/file.t.perm.txt -----x--- good");
  t.ok    (m & S_IROTH,                  "File::mode tmp/file.t.perm.txt ------r-- good");
  t.notok (m & S_IWOTH,                  "File::mode tmp/file.t.perm.txt -------w- good");
  t.notok (m & S_IXOTH,                  "File::mode tmp/file.t.perm.txt --------x good");
  f8.remove ();
  t.notok (f8.exists (),                 "File::remove perm file no longer exists");

  tmp.remove ();
  t.notok (tmp.exists (),                "tmp dir removed.");
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
  umask (0022);
  Directory d10 ("tmp/dir.perm");
  d10.create (0750);
  t.ok (d10.exists (),               "Directory::create perm file exists");
  m = d10.mode ();
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

  // Directory::cd
  Directory d11 ("/tmp");
  t.ok (d11.cd (),                  "Directory::cd /tmp good");

  tmp.remove ();
  t.notok (tmp.exists (),           "tmp dir removed.");

  // File::removeBOM
  std::string line = "Should not be modified.";
  t.is (File::removeBOM (line), line,  "File::removeBOM 'Should not be modified' --> 'Should not be modified'");

  line = "no";
  t.is (File::removeBOM (line), line,  "File::removeBOM 'no' --> 'no'");

  line = "";
  t.is (File::removeBOM (line), line,  "File::removeBOM '' --> ''");

  line = {'\xEF', '\xBB', '\xBF', 'F', 'o', 'o'};
  t.is (File::removeBOM (line), "Foo",  "File::removeBOM '<BOM>Foo' --> 'Foo'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
