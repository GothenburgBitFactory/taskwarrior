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
#include "../auto.h"

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
              << " "
              << context.stringtable.get (CONFIRM_YES_NO, "(y/n)")
              << " ";

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
  options.push_back ("no");
  options.push_back ("All");

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question
              << " ("
              << options[0] << "/"
              << options[1] << "/"
              << options[2]
              << ") ";

    std::getline (std::cin, answer);
    answer = trim (answer);
    autoComplete (answer, options, matches);
  }
  while (matches.size () != 1);

       if (matches[0] == "Yes") return 1;
  else if (matches[0] == "All") return 2;
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
  options.push_back ("no");
  options.push_back ("All");
  options.push_back ("quit");

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question
              << " ("
              << options[0] << "/"
              << options[1] << "/"
              << options[2] << "/"
              << options[3]
              << ") ";

    std::getline (std::cin, answer);
    answer = trim (answer);
    autoComplete (answer, options, matches);
  }
  while (matches.size () != 1);

       if (matches[0] == "Yes")  return 1;
  else if (matches[0] == "All")  return 2;
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
std::string formatSeconds (time_t delta)
{
  char formatted[24];
  float days = (float) delta / 86400.0;

  if (days >= 365)
    sprintf (formatted, "%.1f yrs", (days / 365.2422));   // TODO i18n
  else if (days > 84)
    sprintf (formatted, "%1d mth%s",   // TODO i18n
                        (int) (days / 30.6),
                        ((int) (days / 30.6) == 1 ? "" : "s"));   // TODO i18n
  else if (days > 13)
    sprintf (formatted, "%d wk%s",   // TODO i18n
                        (int) (days / 7.0),
                        ((int) (days / 7.0) == 1 ? "" : "s"));   // TODO i18n
  else if (days >= 1.0)
    sprintf (formatted, "%d day%s",   // TODO i18n
                        (int) days,
                        ((int) days == 1 ? "" : "s"));   // TODO i18n
  else if (days * 24 >= 1.0)
    sprintf (formatted, "%d hr%s",   // TODO i18n
                        (int) (days * 24.0),
                        ((int) (days * 24) == 1 ? "" : "s"));   // TODO i18n
  else if (days * 24 * 60 >= 1)
    sprintf (formatted, "%d min%s",   // TODO i18n
                        (int) (days * 24 * 60),
                        ((int) (days * 24 * 60) == 1 ? "" : "s"));   // TODO i18n
  else if (days * 24 * 60 * 60 >= 1)
    sprintf (formatted, "%d sec%s",   // TODO i18n
                        (int) (days * 24 * 60 * 60),
                        ((int) (days * 24 * 60 * 60) == 1 ? "" : "s"));   // TODO i18n
  else
    strcpy (formatted, "-"); // no i18n

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
std::string formatSecondsCompact (time_t delta)
{
  char formatted[24];
  float days = (float) delta / 86400.0;

  if (days >= 365)                sprintf (formatted, "%.1fy", (days / 365.2422));         // TODO i18n
  else if (days > 84)             sprintf (formatted, "%1dmo", (int) (days / 30.6));       // TODO i18n
  else if (days > 13)             sprintf (formatted, "%dwk", (int) (days / 7.0));         // TODO i18n
  else if (days >= 1.0)           sprintf (formatted, "%dd", (int) days);                  // TODO i18n
  else if (days * 24 >= 1.0)      sprintf (formatted, "%dh", (int) (days * 24.0));         // TODO i18n
  else if (days * 24 * 60 >= 1)   sprintf (formatted, "%dm", (int) (days * 24 * 60));      // TODO i18n
  else if (days * 24 * 3600 >= 1) sprintf (formatted, "%ds", (int) (days * 24 * 60 * 60)); // TODO i18n
  else
    strcpy (formatted, "-"); // no i18n

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
std::string formatBytes (size_t bytes)
{
  char formatted[24];

       if (bytes >=  995000000) sprintf (formatted, "%.1f GiB", (bytes / 1000000000.0));
  else if (bytes >=     995000) sprintf (formatted, "%.1f MiB", (bytes /    1000000.0));
  else if (bytes >=        995) sprintf (formatted, "%.1f KiB", (bytes /       1000.0));
  else                          sprintf (formatted, "%d B", (int)bytes                );

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
      if (length <= item->length () &&
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
// no i18n
std::string expandPath (const std::string& in)
{
  std::string copy = in;
  std::string::size_type tilde;

  if ((tilde = copy.find ("~/")) != std::string::npos)
  {
    struct passwd* pw = getpwuid (getuid ());
    copy.replace (tilde, 1, pw->pw_dir);
  }
  else if ((tilde = copy.find ("~")) != std::string::npos)
  {
    std::string::size_type slash;
    if ((slash = copy.find  ("/", tilde)) != std::string::npos)
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
bool slurp (
  const std::string& file,
  std::vector <std::string>& contents,
  bool trimLines /* = false */)
{
  contents.clear ();

  std::ifstream in (file.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
    {
      if (trimLines) line = trim (line);
      contents.push_back (line);
    }

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool slurp (
  const std::string& file,
  std::string& contents,
  bool trimLines /* = false */)
{
  contents = "";

  std::ifstream in (file.c_str ());
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
    {
      if (trimLines) line = trim (line);
      contents += line + "\n";
    }

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void spit (const std::string& file, const std::string& contents)
{
  std::ofstream out (file.c_str ());
  if (out.good ())
  {
    out << contents;
    out.close ();
  }
  else
    throw std::string ("Could not write file '") + file + "'"; // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
void spit (
  const std::string& file,
  const std::vector <std::string>& lines,
  bool addNewlines /* = true */)
{
  std::ofstream out (file.c_str ());
  if (out.good ())
  {
    foreach (line, lines)
    {
      out << *line;

      if (addNewlines)
        out << "\n";
    }

    out.close ();
  }
  else
    throw std::string ("Could not write file '") + file + "'"; // TODO i18n
}

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

  if (beforeOnly.size () ||
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
        << *name
        << " will be deleted\n";

  foreach (name, afterOnly)
    out << "  - "
        << *name
        << " will be set to '"
        << renderAttribute (*name, after.get (*name))
        << "'\n";

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
      out << "  - "
          << *name
          << " will be changed from '"
          << renderAttribute (*name, before.get (*name))
          << "' to '"
          << renderAttribute (*name, after.get (*name))
          << "'\n";

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "  - No changes will be made\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string renderAttribute (const std::string& name, const std::string& value)
{
  Att a;
  if (a.type (name) == "date")
  {
    Date d ((time_t)::atoi (value.c_str ()));
    return d.toString (context.config.get ("dateformat", "m/d/Y"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
