////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <Directory.h>
#include <Path.h>

// Fixes build with musl libc.
#ifndef GLOB_TILDE
#define GLOB_TILDE 0
#endif

#ifndef GLOB_BRACE
#define GLOB_BRACE 0
#endif

////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& out, const Path& path)
{
  out << path._data;
  return out;
}

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
Path::~Path ()
{
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
    std::string::size_type slash = _data.rfind ('/');
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
    std::string::size_type slash = _data.rfind ('/');
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
    std::string::size_type dot = _data.rfind ('.');
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
  struct stat s = {0};
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
  struct stat s = {0};
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
  std::string expanded = expand (new_name);
  if (_data != expanded)
  {
    if (::rename (_data.c_str (), expanded.c_str ()) == 0)
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

  std::string::size_type tilde = copy.find ("~");
  std::string::size_type slash;

  if (tilde != std::string::npos)
  {
    const char *home = getenv("HOME");
    if (home == NULL)
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
    copy = Directory::cwd () + "/" + in.substr (2);
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
  if (!::glob (pattern.c_str (), GLOB_ERR, NULL, &g))
#else
  if (!::glob (pattern.c_str (), GLOB_ERR | GLOB_BRACE | GLOB_TILDE, NULL, &g))
#endif
    for (int i = 0; i < (int) g.gl_pathc; ++i)
      results.push_back (g.gl_pathv[i]);

  globfree (&g);
  return results;
}

////////////////////////////////////////////////////////////////////////////////
