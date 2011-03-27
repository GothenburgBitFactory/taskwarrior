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
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include "Date.h"
#include "text.h"
#include "main.h"
#include "i18n.h"
#include "util.h"
#include "../cmake.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Uses std::getline, because std::cin eats leading whitespace, and that means
// that if a newline is entered, std::cin eats it and never returns from the
// "std::cin >> answer;" line, but it does display the newline.  This way, with
// std::getline, the newline can be detected, and the prompt re-written.
bool confirm (const std::string& question)
{
  std::string answer;

  do
  {
    std::cout << question
              << " (y/n) ";

    std::getline (std::cin, answer);
    answer = std::cin.eof() ? "no" : lowerCase (trim (answer));
  }
  while (answer != "y"   && // TODO i18n
         answer != "ye"  && // TODO i18n
         answer != "yes" && // TODO i18n
         answer != "n"   && // TODO i18n
         answer != "no");   // TODO i18n

  return (answer == "y" || answer == "ye" || answer == "yes") ? true : false;   // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
// 0 = no
// 1 = yes
// 2 = all
int confirm3 (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back ("Yes");
  options.push_back ("yes");
  options.push_back ("no");
  options.push_back ("All");
  options.push_back ("all");

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question
              << " ("
              << options[1] << "/"
              << options[2] << "/"
              << options[4]
              << ") ";

    std::getline (std::cin, answer);
    answer = trim (answer);
    autoComplete (answer, options, matches);
  }
  while (matches.size () != 1);

       if (matches[0] == "Yes") return 1;
  else if (matches[0] == "yes") return 1;
  else if (matches[0] == "All") return 2;
  else if (matches[0] == "all") return 2;
  else                          return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 0 = no
// 1 = yes
// 2 = all
// 3 = quit
int confirm4 (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back ("Yes");
  options.push_back ("yes");
  options.push_back ("no");
  options.push_back ("All");
  options.push_back ("all");
  options.push_back ("quit");

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question
              << " ("
              << options[1] << "/"
              << options[2] << "/"
              << options[4] << "/"
              << options[5]
              << ") ";

    std::getline (std::cin, answer);
    answer = trim (answer);
    autoComplete (answer, options, matches);
  }
  while (matches.size () != 1);

       if (matches[0] == "Yes")  return 1;
  else if (matches[0] == "yes")  return 1;
  else if (matches[0] == "All")  return 2;
  else if (matches[0] == "all")  return 2;
  else if (matches[0] == "quit") return 3;
  else                           return 0;
}

////////////////////////////////////////////////////////////////////////////////
void delay (float f)
{
  struct timeval t;
  t.tv_sec = (int) f;
  t.tv_usec = int ((f - (int)f) * 1000000);

  select (0, NULL, NULL, NULL, &t);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
std::string formatBytes (size_t bytes)
{
  char formatted[24];

       if (bytes >=  995000000) sprintf (formatted, "%.1f GiB", (bytes / 1000000000.0));
  else if (bytes >=     995000) sprintf (formatted, "%.1f MiB", (bytes /    1000000.0));
  else if (bytes >=        995) sprintf (formatted, "%.1f KiB", (bytes /       1000.0));
  else                          sprintf (formatted, "%d B",     (int)bytes            );

  return commify (formatted);
}

////////////////////////////////////////////////////////////////////////////////
int autoComplete (
  const std::string& partial,
  const std::vector<std::string>& list,
  std::vector<std::string>& matches)
{
  matches.clear ();

  // Handle trivial case. 
  unsigned int length = partial.length ();
  if (length)
  {
    foreach (item, list)
    {
      // An exact match is a special case.  Assume there is only one exact match
      // and return immediately.
      if (partial == *item)
      {
        matches.clear ();
        matches.push_back (*item);
        return 1;
      }

      // Maintain a list of partial matches.
      else if (length <= item->length () &&
               partial == item->substr (0, length))
        matches.push_back (*item);
    }
  }

  return matches.size ();
}

////////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_UUID

#include <uuid/uuid.h>

const std::string uuid ()
{
  uuid_t id;
  uuid_generate (id);
  char buffer[100] = {0};
  uuid_unparse_lower (id, buffer);

  // Bug found by Steven de Brouwer.
  buffer[36] = '\0';

  return std::string (buffer);
}

////////////////////////////////////////////////////////////////////////////////
#else
#include <stdlib.h>
static char randomHexDigit ()
{
  static char digits[] = "0123456789abcdef"; // no i18n
#ifdef HAVE_RANDOM
  // random is better than rand.
  return digits[random () % 16];
#else
  return digits[rand () % 16];
#endif
}

////////////////////////////////////////////////////////////////////////////////
const std::string uuid ()
{
  // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  char id [48] = {0};
  id[0]  = randomHexDigit ();
  id[1]  = randomHexDigit ();
  id[2]  = randomHexDigit ();
  id[3]  = randomHexDigit ();
  id[4]  = randomHexDigit ();
  id[5]  = randomHexDigit ();
  id[6]  = randomHexDigit ();
  id[7]  = randomHexDigit ();
  id[8]  = '-';
  id[9]  = randomHexDigit ();
  id[10] = randomHexDigit ();
  id[11] = randomHexDigit ();
  id[12] = randomHexDigit ();
  id[13] = '-';
  id[14] = randomHexDigit ();
  id[15] = randomHexDigit ();
  id[16] = randomHexDigit ();
  id[17] = randomHexDigit ();
  id[18] = '-';
  id[19] = randomHexDigit ();
  id[20] = randomHexDigit ();
  id[21] = randomHexDigit ();
  id[22] = randomHexDigit ();
  id[23] = '-';
  id[24] = randomHexDigit ();
  id[25] = randomHexDigit ();
  id[26] = randomHexDigit ();
  id[27] = randomHexDigit ();
  id[28] = randomHexDigit ();
  id[29] = randomHexDigit ();
  id[30] = randomHexDigit ();
  id[31] = randomHexDigit ();
  id[32] = randomHexDigit ();
  id[33] = randomHexDigit ();
  id[34] = randomHexDigit ();
  id[35] = randomHexDigit ();
  id[36] = '\0';

  return id;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// On Solaris no flock function exists.
#ifdef SOLARIS
int flock (int fd, int operation)
{
  struct flock fl;

  switch (operation & ~LOCK_NB)
  {
  case LOCK_SH:
    fl.l_type = F_RDLCK;
    break;

  case LOCK_EX:
    fl.l_type = F_WRLCK;
    break;

  case LOCK_UN:
    fl.l_type = F_UNLCK;
    break;

  default:
    errno = EINVAL;
    return -1;
  }

  fl.l_whence = 0;
  fl.l_start  = 0;
  fl.l_len    = 0;

  return fcntl (fd, (operation & LOCK_NB) ? F_SETLK : F_SETLKW, &fl);
}
#endif

////////////////////////////////////////////////////////////////////////////////
bool taskDiff (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  if (beforeOnly.size () !=
      afterOnly.size ())
    return true;

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string taskDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  foreach (name, beforeOnly)
    out << "  - "
        << ucFirst(*name)
        << " will be deleted.\n";

  foreach (name, afterOnly)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << "  - "
          << "Dependencies"
          << " will be set to '"
          << to
          << "'.\n";
    }
    else
      out << "  - "
          << ucFirst(*name)
          << " will be set to '"
          << renderAttribute (*name, after.get (*name))
          << "'.\n";
  }

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << "  - "
            << "Dependencies"
            << " will be changed from '"
            << from
            << "' to '"
            << to
            << "'.\n";
      }
      else
        out << "  - "
            << ucFirst(*name)
            << " will be changed from '"
            << renderAttribute (*name, before.get (*name))
            << "' to '"
            << renderAttribute (*name, after.get (*name))
            << "'.\n";
    }


  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "  - No changes will be made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string taskInfoDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  foreach (name, beforeOnly)
  {
    if (*name == "depends")
    {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        out << "Dependencies '"
            << from
            << "' deleted.\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << "Annotation '"
          << before.get (*name)
          << "' deleted.\n";
    }
    else
    {
      out << ucFirst (*name)
          << " deleted.\n";
    }
  }

  foreach (name, afterOnly)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << "Dependencies"
          << " set to '"
          << to
          << "'.\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << "Annotation of '"
          << after.get (*name)
          << "' added.\n";
    }
    else
      out << ucFirst(*name)
          << " set to '"
          << renderAttribute (*name, after.get (*name))
          << "'.\n";
  }

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name) &&
        before.get (*name) != "" && after.get (*name) != "")
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << "Dependencies"
            << " changed from '"
            << from
            << "' to '"
            << to
            << "'.\n";
      }
      else if (name->substr (0, 11) == "annotation_")
      {
        out << "Annotation changed to '"
            << after.get (*name)
            << "'.\n";
      }
      else
        out << ucFirst(*name)
            << " changed from '"
            << renderAttribute (*name, before.get (*name))
            << "' to '"
            << renderAttribute (*name, after.get (*name))
            << "'.\n";
    }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "No changes made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string renderAttribute (const std::string& name, const std::string& value)
{
  Att a;
  if (a.type (name) == "date" &&
      value != "")
  {
    Date d ((time_t)::atoi (value.c_str ()));
    return d.toString (context.config.get ("dateformat"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// The vector must be sorted first.  This is a modified version of the run-
// length encoding algorithm.
//
// This function converts the vector:
//
//   [1, 3, 4, 6, 7, 8, 9, 11]
//
// to ths string:
//
//   1,3-4,6-9,11
//
std::string compressIds (const std::vector <int>& ids)
{
  std::stringstream result;

  int range_start = 0;
  int range_end = 0;

  for (int i = 0; i < ids.size (); ++i)
  {
    if (i + 1 == ids.size ())
    {
      if (result.str ().length ())
        result << ",";

      if (range_start < range_end)
        result << ids[range_start] << "-" << ids[range_end];
      else
        result << ids[range_start];
    }
    else
    {
      if (ids[range_end] + 1 == ids[i + 1])
      {
        ++range_end;
      }
      else
      {
        if (result.str ().length ())
          result << ",";

        if (range_start < range_end)
          result << ids[range_start] << "-" << ids[range_end];
        else
          result << ids[range_start];

        range_start = range_end = i + 1;
      }
    }
  }

  return result.str ();
}

////////////////////////////////////////////////////////////////////////////////
