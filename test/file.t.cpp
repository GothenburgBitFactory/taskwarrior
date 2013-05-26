////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <File.h>
#include <Directory.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (15);

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

  tmp.remove ();
  t.notok (tmp.exists (),                "tmp dir removed.");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
