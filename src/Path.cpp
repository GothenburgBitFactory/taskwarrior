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
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include "Path.h"

////////////////////////////////////////////////////////////////////////////////
Path::Path ()
{
}

////////////////////////////////////////////////////////////////////////////////
Path::Path (const Path& other)
{
  if (this != &other)
    data = other.data;
}

////////////////////////////////////////////////////////////////////////////////
Path::Path (const std::string& in)
{
  data = expand (in);
}

////////////////////////////////////////////////////////////////////////////////
Path::~Path ()
{
}

////////////////////////////////////////////////////////////////////////////////
Path& Path::operator= (const Path& other)
{
  if (this != &other)
    this->data = other.data;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::name () const
{
  if (data.length ())
  {
    std::string::size_type slash = data.rfind ('/');
    if (slash != std::string::npos)
      return data.substr (slash + 1, std::string::npos);
  }

 return data;
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::parent () const
{
  if (data.length ())
  {
    std::string::size_type slash = data.rfind ('/');
    if (slash != std::string::npos)
      return data.substr (0, slash);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Path::extension () const
{
  if (data.length ())
  {
    std::string::size_type dot = data.rfind ('.');
    if (dot != std::string::npos)
      return data.substr (dot + 1, std::string::npos);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool Path::exists () const
{
  return access (data.c_str (), F_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::is_directory () const
{
  struct stat s = {0};
  if (! stat (data.c_str (), &s) &&
      s.st_mode & S_IFDIR)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::readable () const
{
  return access (data.c_str (), R_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::writable () const
{
  return access (data.c_str (), W_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
bool Path::executable () const
{
  return access (data.c_str (), X_OK) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
// ~      --> /home/user
// ~foo/x --> /home/foo/s
// ~/x    --> /home/foo/x
std::string Path::expand (const std::string& in)
{
  std::string copy = in;

  std::string::size_type tilde = copy.find ("~");
  std::string::size_type slash;

  if (tilde != std::string::npos)
  {
    struct passwd* pw = getpwuid (getuid ());

    // Convert: ~ --> /home/user
    if (copy.length () == 1)
    {
      copy = pw->pw_dir;
    }

    // Convert: ~/x --> /home/user/x
    else if (copy.length () > tilde + 1 &&
             copy[tilde + 1] == '/')
    {
      copy.replace (tilde, 1, pw->pw_dir);
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

  return copy;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Path::glob (const std::string& pattern)
{
  std::vector <std::string> results;

  glob_t g;
  if (!::glob (pattern.c_str (), GLOB_ERR | GLOB_BRACE | GLOB_TILDE, NULL, &g))
    for (int i = 0; i < (int) g.gl_pathc; ++i)
      results.push_back (g.gl_pathv[i]);

  globfree (&g);
  return results;
}

////////////////////////////////////////////////////////////////////////////////
