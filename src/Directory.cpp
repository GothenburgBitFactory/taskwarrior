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

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <Directory.h>
#include <cmake.h>

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
Directory::~Directory ()
{
}

////////////////////////////////////////////////////////////////////////////////
Directory& Directory::operator= (const Directory& other)
{
  if (this != &other)
  {
    File::operator= (other);
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::create ()
{
  return mkdir (_data.c_str (), 0755) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove ()
{
  return remove_directory (_data);
}

////////////////////////////////////////////////////////////////////////////////
bool Directory::remove_directory (const std::string& dir)
{
  DIR* dp = opendir (dir.c_str ());
  if (dp != NULL)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != NULL)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      stat (de->d_name, &s);
      if (s.st_mode & S_IFDIR)
        remove_directory (dir + "/" + de->d_name);
      else
        unlink ((dir + "/" + de->d_name).c_str ());
#else
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
void Directory::list (
  const std::string& base,
  std::vector <std::string>& results,
  bool recursive)
{
  DIR* dp = opendir (base.c_str ());
  if (dp != NULL)
  {
    struct dirent* de;
    while ((de = readdir (dp)) != NULL)
    {
      if (!strcmp (de->d_name, ".") ||
          !strcmp (de->d_name, ".."))
        continue;

#if defined (SOLARIS) || defined (HAIKU)
      struct stat s;
      stat (de->d_name, &s);
      if (recursive && s.st_mode & S_IFDIR)
        list (base + "/" + de->d_name, results, recursive);
      else
        results.push_back (base + "/" + de->d_name);
#else
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
