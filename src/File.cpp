////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include <fstream>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "File.h"

////////////////////////////////////////////////////////////////////////////////
File::File ()
: Path::Path ()
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const Path& other)
: Path::Path (other)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const File& other)
: Path::Path (other)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const std::string& in)
: Path::Path (in)
{
}

////////////////////////////////////////////////////////////////////////////////
File::~File ()
{
}

////////////////////////////////////////////////////////////////////////////////
File& File::operator= (const File& other)
{
  if (this != &other)
  {
    Path::operator= (other);
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool File::create ()
{
  std::ofstream out (data.c_str ());
  if (out.good ())
  {
    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::remove ()
{
  return unlink (data.c_str ()) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
//  S_IFMT          0170000  type of file
//         S_IFIFO  0010000  named pipe (fifo)
//         S_IFCHR  0020000  character special
//         S_IFDIR  0040000  directory
//         S_IFBLK  0060000  block special
//         S_IFREG  0100000  regular
//         S_IFLNK  0120000  symbolic link
//         S_IFSOCK 0140000  socket
//         S_IFWHT  0160000  whiteout
//  S_ISUID         0004000  set user id on execution
//  S_ISGID         0002000  set group id on execution
//  S_ISVTX         0001000  save swapped text even after use
//  S_IRUSR         0000400  read permission, owner
//  S_IWUSR         0000200  write permission, owner
//  S_IXUSR         0000100  execute/search permission, owner
mode_t File::mode ()
{
  struct stat s;
  if (!stat (data.c_str (), &s))
    return s.st_mode;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
size_t File::size () const
{
  struct stat s;
  if (!stat (data.c_str (), &s))
    return s.st_size;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool File::create (const std::string& name)
{
  std::ofstream out (expand (name).c_str ());
  if (out.good ())
  {
    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string File::read (const std::string& name)
{
  std::string contents = "";

  std::ifstream in (name.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
      contents += line + "\n";

    in.close ();
  }

  return contents;
}

////////////////////////////////////////////////////////////////////////////////
bool File::read (const std::string& name, std::string& contents)
{
  contents = "";

  std::ifstream in (name.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
      contents += line + "\n";

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::read (const std::string& name, std::vector <std::string>& contents)
{
  contents.clear ();

  std::ifstream in (name.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
      contents.push_back (line);

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::write (const std::string& name, const std::string& contents)
{
  std::ofstream out (expand (name).c_str (),
                     std::ios_base::out | std::ios_base::trunc);
  if (out.good ())
  {
    out << contents;
    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::write (
  const std::string& name,
  const std::vector <std::string>& lines,
  bool addNewlines /* = true */)
{
  std::ofstream out (expand (name).c_str (),
                     std::ios_base::out | std::ios_base::trunc);
  if (out.good ())
  {
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      out << *it;

      if (addNewlines)
        out << "\n";
    }

    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::append (const std::string& name, const std::string& contents)
{
  std::ofstream out (expand (name).c_str (),
                     std::ios_base::out | std::ios_base::app);
  if (out.good ())
  {
    out << contents;
    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::append (
  const std::string& name,
  const std::vector <std::string>& lines,
  bool addNewlines /* = true */)
{
  std::ofstream out (expand (name).c_str (),
                     std::ios_base::out | std::ios_base::app);
  if (out.good ())
  {
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
    {
      out << *it;

      if (addNewlines)
        out << "\n";
    }

    out.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::remove (const std::string& name)
{
  return unlink (expand (name).c_str ()) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

