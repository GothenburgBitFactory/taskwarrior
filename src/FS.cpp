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
#include <FS.h>
#include <fstream>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

#if defined SOLARIS || defined NETBSD || defined FREEBSD
#include <limits.h>
#endif

#if defined __APPLE__
#include <sys/syslimits.h>
#endif

// Fixes build with musl libc.
#ifndef GLOB_TILDE
#define GLOB_TILDE 0
#endif

#ifndef GLOB_BRACE
#define GLOB_BRACE 0
#endif

////////////////////////////////////////////////////////////////////////////////
Path::Path ()
{
}

////////////////////////////////////////////////////////////////////////////////
Path::Path (const Path& other)
{
  if (this != &other)
  {
    _original = other._original;
    _data     = other._data;
  }
}

////////////////////////////////////////////////////////////////////////////////
Path::Path (const std::string& in)
{
  _original = in;
  _data = expand (in);
}

////////////////////////////////////////////////////////////////////////////////
Path& Path::operator= (const Path& other)
{
  if (this != &other)
  {
    this->_original = other._original;
    this->_data     = other._data;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::operator== (const Path& other)
{
  return _data == other._data;
}

////////////////////////////////////////////////////////////////////////////////
Path& Path::operator+= (const std::string& dir)
{
  _data += "/" + dir;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Path::operator std::string () const
{
  return _data;
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::name () const
{
  if (_data.length ())
  {
    auto slash = _data.rfind ('/');
    if (slash != std::string::npos)
      return _data.substr (slash + 1, std::string::npos);
  }

 return _data;
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::parent () const
{
  if (_data.length ())
  {
    auto slash = _data.rfind ('/');
    if (slash != std::string::npos)
      return _data.substr (0, slash);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::extension () const
{
  if (_data.length ())
  {
    auto dot = _data.rfind ('.');
    if (dot != std::string::npos)
      return _data.substr (dot + 1, std::string::npos);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool Path::exists () const
{
  return access (_data.c_str (), F_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::is_directory () const
{
  struct stat s {};
  if (! stat (_data.c_str (), &s) &&
      S_ISDIR (s.st_mode))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::is_absolute () const
{
  if (_data.length () && _data[0] == '/')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::is_link () const
{
  struct stat s {};
  if (! lstat (_data.c_str (), &s) &&
      S_ISLNK (s.st_mode))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::readable () const
{
  return access (_data.c_str (), R_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::writable () const
{
  return access (_data.c_str (), W_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::executable () const
{
  return access (_data.c_str (), X_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::rename (const std::string& new_name)
{
  auto expanded = expand (new_name);
  if (_data != expanded)
  {
    if (std::rename (_data.c_str (), expanded.c_str ()) == 0)
    {
      _data = expanded;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ~      --> /home/user
// ~foo/x --> /home/foo/s
// ~/x    --> /home/foo/x
// ./x    --> $PWD/x
// x      --> $PWD/x
std::string Path::expand (const std::string& in)
{
  std::string copy = in;

  auto tilde = copy.find ("~");
  std::string::size_type slash;

  if (tilde != std::string::npos)
  {
    const char *home = getenv("HOME");
    if (home == nullptr)
    {
      struct passwd* pw = getpwuid (getuid ());
      home = pw->pw_dir;
    }

    // Convert: ~ --> /home/user
    if (copy.length () == 1)
      copy = home;

    // Convert: ~/x --> /home/user/x
    else if (copy.length () > tilde + 1 &&
             copy[tilde + 1] == '/')
    {
      copy.replace (tilde, 1, home);
    }

    // Convert: ~foo/x --> /home/foo/x
    else if ((slash = copy.find  ("/", tilde)) != std::string::npos)
    {
      std::string name = copy.substr (tilde + 1, slash - tilde - 1);
      struct passwd* pw = getpwnam (name.c_str ());
      if (pw)
        copy.replace (tilde, slash - tilde, pw->pw_dir);
    }
  }

  // Relative paths
  else if (in.length () > 2 &&
           in.substr (0, 2) == "./")
  {
    copy = Directory::cwd () + in.substr (1);
  }
  else if (in.length () > 1 &&
           in[0] != '.' &&
           in[0] != '/')
  {
    copy = Directory::cwd () + "/" + in;
  }

  return copy;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Path::glob (const std::string& pattern)
{
  std::vector <std::string> results;

  glob_t g;
#ifdef SOLARIS
  if (!::glob (pattern.c_str (), GLOB_ERR, nullptr, &g))
#else
  if (!::glob (pattern.c_str (), GLOB_ERR | GLOB_BRACE | GLOB_TILDE, nullptr, &g))
#endif
    for (int i = 0; i < (int) g.gl_pathc; ++i)
      results.push_back (g.gl_pathv[i]);

  globfree (&g);
  return results;
}

////////////////////////////////////////////////////////////////////////////////
File::File ()
: Path::Path ()
, _fh (nullptr)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const Path& other)
: Path::Path (other)
, _fh (nullptr)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const File& other)
: Path::Path (other)
, _fh (nullptr)
, _h (-1)
, _locked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
File::File (const std::string& in)
: Path::Path (in)
, _fh (nullptr)
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
std::string File::removeBOM (const std::string& input)
{
  if (input[0] && input[0] == '\xEF' &&
      input[1] && input[1] == '\xBB' &&
      input[2] && input[2] == '\xBF')
    return input.substr (3);

  return input;
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
void File::close ()
{
  if (_fh)
  {
    if (_locked)
      unlock ();

    fclose (_fh);
    _fh = nullptr;
    _h = -1;
    _locked = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
bool File::lock ()
{
  _locked = false;
  if (_fh && _h != -1)
  {
                    // l_type   l_whence  l_start  l_len  l_pid
    struct flock fl = {F_WRLCK, SEEK_SET, 0,       0,     0 };
    fl.l_pid = getpid ();
    if (fcntl (_h, F_SETLKW, &fl) == 0)
      _locked = true;
  }

  return _locked;
}

////////////////////////////////////////////////////////////////////////////////
void File::unlock ()
{
  if (_locked)
  {
                    // l_type   l_whence  l_start  l_len  l_pid
    struct flock fl = {F_UNLCK, SEEK_SET, 0,       0,     0 };
    fl.l_pid = getpid ();

    fcntl (_h, F_SETLK, &fl);
    _locked = false;
  }
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
    bool first = true;
    std::string line;
    line.reserve (512 * 1024);
    while (getline (in, line))
    {
      // Detect forbidden BOM on first line.
      if (first)
      {
        line = File::removeBOM (line);
        first = false;
      }

      contents += line + "\n";
    }

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
    bool first = true;
    std::string line;
    line.reserve (512 * 1024);
    while (getline (in, line))
    {
      // Detect forbidden BOM on first line.
      if (first)
      {
        line = File::removeBOM (line);
        first = false;
      }

      contents.push_back (line);
    }

    in.close ();
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
    for (auto& line : lines)
      fputs (line.c_str (), _fh);
  }
}

////////////////////////////////////////////////////////////////////////////////
void File::write_raw (const std::string& line)
{
  if (!_fh)
    open ();

  if (_fh)
    fputs (line.c_str (), _fh);
}

////////////////////////////////////////////////////////////////////////////////
void File::truncate ()
{
  if (!_fh)
    open ();

  if (_fh)
    (void) ftruncate (_h, 0);
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
bool File::read (const std::string& name, std::string& contents)
{
  contents = "";

  std::ifstream in (name.c_str ());
  if (in.good ())
  {
    bool first = true;
    std::string line;
    line.reserve (1024);
    while (getline (in, line))
    {
      // Detect forbidden BOM on first line.
      if (first)
      {
        line = File::removeBOM (line);
        first = false;
      }

      contents += line + "\n";
    }

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
    bool first = true;
    std::string line;
    line.reserve (1024);
    while (getline (in, line))
    {
      // Detect forbidden BOM on first line.
      if (first)
      {
        line = File::removeBOM (line);
        first = false;
      }

      contents.push_back (line);
    }

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
    for (auto& line : lines)
    {
      out << line;

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
bool File::copy (const std::string& from, const std::string& to)
{
  // 'from' must exist.
  if (! access (from.c_str (), F_OK))
  {
    std::ifstream src (from, std::ios::binary);
    std::ofstream dst (to,   std::ios::binary);

    dst << src.rdbuf ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool File::move (const std::string& from, const std::string& to)
{
  auto expanded = expand (to);
  if (from != expanded)
    if (std::rename (from.c_str (), to.c_str ()) == 0)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory ()
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const Directory& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const File& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const Path& other)
: File::File (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory (const std::string& in)
: File::File (in)
{
}

////////////////////////////////////////////////////////////////////////////////
Directory& Directory::operator= (const Directory& other)
{
  if (this != &other)
    File::operator= (other);

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::create (int mode /* = 0755 */)
{
  return mkdir (_data.c_str (), mode) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove () const
{
  return remove_directory (_data);
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove_directory (const std::string& dir) const
{
  DIR* dp = opendir (dir.c_str ());
  if (dp != nullptr)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != nullptr)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      lstat ((dir + "/" + de->d_name).c_str (), &s);
      if (S_ISDIR (s.st_mode))
        remove_directory (dir + "/" + de->d_name);
      else
        unlink ((dir + "/" + de->d_name).c_str ());
#else
      if (de->d_type == DT_UNKNOWN)
      {
        struct stat s;
        lstat ((dir + "/" + de->d_name).c_str (), &s);
        if (S_ISDIR (s.st_mode))
          de->d_type = DT_DIR;
      }
      if (de->d_type == DT_DIR)
        remove_directory (dir + "/" + de->d_name);
      else
        unlink ((dir + "/" + de->d_name).c_str ());
#endif
    }

    closedir (dp);
  }

  return rmdir (dir.c_str ()) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Directory::list ()
{
  std::vector <std::string> files;
  list (_data, files, false);
  return files;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Directory::listRecursive ()
{
  std::vector <std::string> files;
  list (_data, files, true);
  return files;
}

////////////////////////////////////////////////////////////////////////////////
std::string Directory::cwd ()
{
#ifdef HAVE_GET_CURRENT_DIR_NAME
  char *buf = get_current_dir_name ();
  if (buf == nullptr)
    throw std::bad_alloc ();
  std::string result (buf);
  free (buf);
  return result;
#else
  char buf[PATH_MAX];
  getcwd (buf, PATH_MAX - 1);
  return std::string (buf);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::up ()
{
  if (_data == "/")
    return false;

  auto slash = _data.rfind ('/');
  if (slash == 0)
  {
    _data = "/";  // Root dir should retain the slash.
    return true;
  }
  else if (slash != std::string::npos)
  {
    _data = _data.substr (0, slash);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::cd () const
{
  return chdir (_data.c_str ()) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void Directory::list (
  const std::string& base,
  std::vector <std::string>& results,
  bool recursive)
{
  DIR* dp = opendir (base.c_str ());
  if (dp != nullptr)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != nullptr)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      stat ((base + "/" + de->d_name).c_str (), &s);
      if (recursive && S_ISDIR (s.st_mode))
        list (base + "/" + de->d_name, results, recursive);
      else
        results.push_back (base + "/" + de->d_name);
#else
      if (recursive && de->d_type == DT_UNKNOWN)
      {
        struct stat s;
        lstat ((base + "/" + de->d_name).c_str (), &s);
        if (S_ISDIR (s.st_mode))
          de->d_type = DT_DIR;
      }
      if (recursive && de->d_type == DT_DIR)
        list (base + "/" + de->d_name, results, recursive);
      else
        results.push_back (base + "/" + de->d_name);
#endif
    }

    closedir (dp);
  }
}

////////////////////////////////////////////////////////////////////////////////
