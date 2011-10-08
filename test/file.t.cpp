////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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


#include <Context.h>
#include <File.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest t (6);

  File::write ("/tmp/file.t.txt", "This is a test\n");
  File f6 ("/tmp/file.t.txt");
	t.ok (f6.size () == 15, "File::size /tmp/file.t.txt good");
  t.ok (f6.mode () & S_IRUSR, "File::mode /tmp/file.t.txt good");
  t.ok (File::remove ("/tmp/file.t.txt"), "File::remove /tmp/file.t.txt good");

  // operator (std::string) const;
  t.is ((std::string) f6, "/tmp/file.t.txt", "File::operator (std::string) const");

  t.ok (File::create ("/tmp/file.t.create"), "File::create /tmp/file.t.create good");
  t.ok (File::remove ("/tmp/file.t.create"), "File::remove /tmp/file.t.create good");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
