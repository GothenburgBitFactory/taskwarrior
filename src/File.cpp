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
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef SOLARIS
#include <fcntl.h> // for flock() replacement
#include <string.h> // for memset()
#else
#include <sys/file.h>
#endif
#include <pwd.h>
#include <unistd.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
File::File ()
: Path::Path ()
, _fh (NULL)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const Path& other)
: Path::Path (other)
, _fh (NULL)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const File& other)
: Path::Path (other)
, _fh (NULL)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const std::string& in)
: Path::Path (in)
, _fh (NULL)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::~File ()
{
  if (_fh)
    close ();
}

////////////////////////////////////////////////////////////////////////////////
File& File::operator= (const File& other)
{
  if (this != &other)
    Path::operator= (other);

  _locked = false;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool File::create (int mode /* = 0640 */)
{
  if (open ())
  {
    fchmod (_h, mode);
    close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::remove () const
{
  return unlink (_data.c_str ()) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::open ()
{
  if (_data != "")
  {
    if (! _fh)
    {
      bool already_exists = exists ();
      if (already_exists)
        if (!readable () || !writable ())
          throw std::string (format (STRING_FILE_PERMS, _data));

      _fh = fopen (_data.c_str (), (already_exists ? "r+" : "w+"));
      if (_fh)
      {
        _h = fileno (_fh);
        _locked = false;
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
  if (_fh)
  {
    fclose (_fh);
    _fh = NULL;
    _h = -1;
    _locked = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
bool File::lock ()
{
  if (_fh && _h != -1)
  {
    // Try three times before failing.
    int retry = 0;
    while (flock (_h, LOCK_NB | LOCK_EX) && ++retry <= 3)
      ;

    if (retry <= 3)
    {
      _locked = true;
      return true;
    }
  }

  _locked = false;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::waitForLock ()
{
  if (_locked)
    return true;

  if (_fh && _h != -1)
    if (flock (_h, LOCK_EX) == 0)
    {
      _locked = true;
      return true;
    }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::read (std::string& contents)
{
  contents = "";
  contents.reserve (size ());

  std::ifstream in (_data.c_str ());
  if (in.good ())
  {
    std::string line;
    line.reserve (512 * 1024);
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

  std::ifstream in (_data.c_str ());
  if (in.good ())
  {
    std::string line;
    line.reserve (512 * 1024);
    while (getline (in, line))
      contents.push_back (line);

    in.close ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::write (const std::string& line)
{
  if (!_fh)
    open ();

  if (_fh)
    fputs (line.c_str (), _fh);
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::write (const std::vector <std::string>& lines)
{
  if (!_fh)
    open ();

  if (_fh)
  {
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
      fputs (it->c_str (), _fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::append (const std::string& line)
{
  if (!_fh)
    open ();

  if (_fh)
  {
    fseek (_fh, 0, SEEK_END);
    fputs (line.c_str (), _fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Opens if necessary.
void File::append (const std::vector <std::string>& lines)
{
  if (!_fh)
    open ();

  if (_fh)
  {
    fseek (_fh, 0, SEEK_END);
    std::vector <std::string>::const_iterator it;
    for (it = lines.begin (); it != lines.end (); ++it)
      fputs (((*it) + "\n").c_str (), _fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
void File::truncate ()
{
  if (!_fh)
    open ();

  if (_fh)
    ftruncate (_h, 0);
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
  if (!stat (_data.c_str (), &s))
    return s.st_mode;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
size_t File::size () const
{
  struct stat s;
  if (!stat (_data.c_str (), &s))
    return s.st_size;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
time_t File::mtime () const
{
  struct stat s;
  if (!stat (_data.c_str (), &s))
    return s.st_mtime;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
time_t File::ctime () const
{
  struct stat s;
  if (!stat (_data.c_str (), &s))
    return s.st_ctime;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
time_t File::btime () const
{
  struct stat s;
  if (!stat (_data.c_str (), &s))
#ifdef HAVE_ST_BIRTHTIME
    return s.st_birthtime;
#else
    return s.st_ctime;
#endif

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool File::create (const std::string& name, int mode /* = 0640 */)
{
  std::string full_name = expand (name);
  std::ofstream out (full_name.c_str ());
  if (out.good ())
  {
    out.close ();
    chmod (full_name.c_str (), mode);
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
    line.reserve (1024);
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
    line.reserve (1024);
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
    line.reserve (1024);
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

