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
