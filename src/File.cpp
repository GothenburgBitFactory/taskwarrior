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

#define L10N                                           // Localization complete.

#include <fstream>
#include <sys/types.h>
#include <sys/file.h>
#include <pwd.h>
#include <unistd.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
File::File ()
: Path::Path ()
, fh (NULL)
, h (-1)
, locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const Path& other)
: Path::Path (other)
, fh (NULL)
, h (-1)
, locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const File& other)
: Path::Path (other)
, fh (NULL)
, h (-1)
, locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const std::string& in)
: Path::Path (in)
, fh (NULL)
, h (-1)
, locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::~File ()
{
  if (fh)
    close ();
}

////////////////////////////////////////////////////////////////////////////////
File& File::operator= (const File& other)
{
  if (this != &other)
    Path::operator= (other);

  locked = false;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool File::create ()
{
  if (open ())
  {
    close ();
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
bool File::open ()
{
  if (data != "")
  {
    if (! fh)
    {
      bool already_exists = exists ();
      if (already_exists)
        if (!readable () || !writable ())
          throw std::string (format (STRING_FILE_PERMS, data));

      fh = fopen (data.c_str (), (already_exists ? "r+" : "w+"));
      if (fh)
      {
        h = fileno (fh);
        locked = false;
        return true;
      }
    }
    else
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::openAndLock ()
{
  return open () && lock ();
}

////////////////////////////////////////////////////////////////////////////////
void File::close ()
{
  if (fh)
  {
    fclose (fh);
    fh = NULL;
    h = -1;
    locked = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
bool File::lock ()
{
  if (fh && h != -1)
  {
    // Try three times before failing.
    int retry = 0;
    while (flock (h, LOCK_NB | LOCK_EX) && ++retry <= 3)
      ;

    if (retry <= 3)
    {
      locked = true;
      return true;
    }
  }

  locked = false;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::waitForLock ()
{
  if (locked)
    return true;

  if (fh && h != -1)
    if (flock (h, LOCK_EX) == 0)
    {
      locked = true;
      return true;
    }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::read (std::string& contents)
{
  contents = "";

  std::ifstream in (data.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
      contents += line + "\n";

    in.close ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::read (std::vector <std::string>& contents)
{
  contents.clear ();

  std::ifstream in (data.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
      contents.push_back (line);

    in.close ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::write (const std::string& line)
{
  if (!fh)
    open ();

  if (fh)
    fputs (line.c_str (), fh);
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::write (const std::vector <std::string>& lines)
{
  if (!fh)
    open ();

  if (fh)
  {
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
      fputs (it->c_str (), fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::append (const std::string& line)
{
  if (!fh)
    open ();

  if (fh)
  {
    fseek (fh, 0, SEEK_END);
    fputs (line.c_str (), fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::append (const std::vector <std::string>& lines)
{
  if (!fh)
    open ();

  if (fh)
  {
    fseek (fh, 0, SEEK_END);
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
      fputs (((*it) + "\n").c_str (), fh);
  }
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
time_t File::mtime () const
{
  struct stat s;
  if (!stat (data.c_str (), &s))
    return s.st_mtime;

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

