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
#include <limits>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
// If <iostream> is included, put it after <stdio.h>, because it includes
// <stdio.h>, and therefore would ignore the _WITH_GETLINE.
#ifdef FREEBSD
#define _WITH_GETLINE
#endif
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include <Date.h>
#include <text.h>
#include <main.h>
#include <i18n.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static inline unsigned round_up_to (unsigned n, unsigned target)
{
  return n + target - (n % target);
}

////////////////////////////////////////////////////////////////////////////////
// Uses std::getline, because std::cin eats leading whitespace, and that means
// that if a newline is entered, std::cin eats it and never returns from the
// "std::cin >> answer;" line, but it does display the newline.  This way, with
// std::getline, the newline can be detected, and the prompt re-written.
bool confirm (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back (STRING_UTIL_CONFIRM_YES);
  options.push_back (STRING_UTIL_CONFIRM_NO);

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question
              << STRING_UTIL_CONFIRM_YN;

    std::getline (std::cin, answer);
    answer = std::cin.eof() ? STRING_UTIL_CONFIRM_NO : lowerCase (trim (answer));

    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (matches.size () != 1);

  return matches[0] == STRING_UTIL_CONFIRM_YES ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// 0 = no
// 1 = yes
// 2 = all
int confirm3 (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back (STRING_UTIL_CONFIRM_YES_U);
  options.push_back (STRING_UTIL_CONFIRM_YES);
  options.push_back (STRING_UTIL_CONFIRM_NO);
  options.push_back (STRING_UTIL_CONFIRM_ALL_U);
  options.push_back (STRING_UTIL_CONFIRM_ALL);

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
    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (matches.size () != 1);

       if (matches[0] == STRING_UTIL_CONFIRM_YES_U) return 1;
  else if (matches[0] == STRING_UTIL_CONFIRM_YES)   return 1;
  else if (matches[0] == STRING_UTIL_CONFIRM_ALL_U) return 2;
  else if (matches[0] == STRING_UTIL_CONFIRM_ALL)   return 2;
  else                                              return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 0 = no
// 1 = yes
// 2 = all
// 3 = quit
int confirm4 (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back (STRING_UTIL_CONFIRM_YES_U);
  options.push_back (STRING_UTIL_CONFIRM_YES);
  options.push_back (STRING_UTIL_CONFIRM_NO);
  options.push_back (STRING_UTIL_CONFIRM_ALL_U);
  options.push_back (STRING_UTIL_CONFIRM_ALL);
  options.push_back (STRING_UTIL_CONFIRM_QUIT);

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
    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (matches.size () != 1);

       if (matches[0] == STRING_UTIL_CONFIRM_YES_U) return 1;
  else if (matches[0] == STRING_UTIL_CONFIRM_YES)   return 1;
  else if (matches[0] == STRING_UTIL_CONFIRM_ALL_U) return 2;
  else if (matches[0] == STRING_UTIL_CONFIRM_ALL)   return 2;
  else if (matches[0] == STRING_UTIL_CONFIRM_QUIT)  return 3;
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

       if (bytes >=  995000000) sprintf (formatted, "%.1f %s", (bytes / 1000000000.0), STRING_UTIL_GIBIBYTES);
  else if (bytes >=     995000) sprintf (formatted, "%.1f %s", (bytes /    1000000.0), STRING_UTIL_MEBIBYTES);
  else if (bytes >=        995) sprintf (formatted, "%.1f %s", (bytes /       1000.0), STRING_UTIL_KIBIBYTES);
  else                          sprintf (formatted, "%d %s",   (int)bytes,             STRING_UTIL_BYTES);

  return commify (formatted);
}

////////////////////////////////////////////////////////////////////////////////
int autoComplete (
  const std::string& partial,
  const std::vector<std::string>& list,
  std::vector<std::string>& matches,
  int minimum/* = 1*/)
{
  matches.clear ();

  // Handle trivial case. 
  unsigned int length = partial.length ();
  if (length)
  {
    std::vector <std::string>::const_iterator item;
    for (item = list.begin (); item != list.end (); ++item)
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
      else if (length >= (unsigned) minimum &&
               length <= item->length ()    &&
               partial == item->substr (0, length))
        matches.push_back (*item);
    }
  }

  return matches.size ();
}

// Handle the generation of UUIDs on FreeBSD in a separate implementation
// of the uuid () function, since the API is quite different from Linux's.
// Also, uuid_unparse_lower is not needed on FreeBSD, because the string
// representation is always lowercase anyway.
// For the implementation details, refer to
// http://svnweb.freebsd.org/base/head/sys/kern/kern_uuid.c
#ifdef FREEBSD
const std::string uuid ()
{
  uuid_t id;
  uint32_t status;
  char *buffer (0);
  uuid_create (&id, &status);
  uuid_to_string (&id, &buffer, &status);

  std::string res (buffer);
  free (buffer);

  return res;
}
#else

////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_UUID_UNPARSE_LOWER
// Older versions of libuuid don't have uuid_unparse_lower(), only uuid_unparse()
void uuid_unparse_lower (uuid_t uu, char *out)
{
    uuid_unparse (uu, out);
    // Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
    // a-z by bitwise or with 0x20, and the others already have this bit set
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}
#endif

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

  for (unsigned int i = 0; i < ids.size (); ++i)
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
void combine (std::vector <int>& dest, const std::vector <int>& source)
{
  // Create a map using the sequence elements as keys.  This will create a
  // unique list, with no duplicates.
  std::map <int, int> both;
  std::vector <int>::iterator i1;
  for (i1 = dest.begin (); i1 != dest.end (); ++i1)
    both[*i1] = 0;

  std::vector <int>::const_iterator i2;
  for (i2 = source.begin (); i2 != source.end (); ++i2)
    both[*i2] = 0;

  // Now make a sequence out of the keys of the map.
  dest.clear ();
  std::map <int, int>::iterator i3;
  for (i3 = both.begin (); i3 != both.end (); ++i3)
    dest.push_back (i3->first);

  std::sort (dest.begin (), dest.end ());
}

////////////////////////////////////////////////////////////////////////////////
// Run a binary with args, capturing output.
int execute (
  const std::string& executable,
  const std::vector <std::string>& args,
  const std::string& input,
  std::string& output)
{
  int pin[2], pout[2];
  pipe (pin);
  pipe (pout);

  pid_t pid = fork();
  if (!pid)
  {
    // This is only reached in the child
    dup2 (pin[0], STDIN_FILENO);
    dup2 (pout[1], STDOUT_FILENO);

    char** argv = new char*[args.size () + 1];
    for (unsigned int i = 0; i < args.size (); ++i)
      argv[i] = (char*) args[i].c_str ();

    argv[args.size ()] = NULL;

    int ret = execvp (executable.c_str (), argv);
    delete[] argv;
    exit (ret);
  }

  // This is only reached in the parent
  close (pin[0]);
  close (pout[1]);

  // Write input to fp.
  FILE* pinf = fdopen (pin[1], "w");
  if (input != "" &&
      input != "\n")
  {
    fputs (input.c_str (), pinf);
  }

  fclose (pinf);
  close (pin[1]);

  // Read output from fp.
  output = "";
  char* line = NULL;
  size_t len = 0;
  FILE* poutf = fdopen(pout[0], "r");
  while (getline (&line, &len, poutf) != -1)
    output += line;

  free (line);
  line = NULL;
  fclose (poutf);
  close (pout[0]);

  int status = -1;
  wait (&status);
  if (WIFEXITED (status))
    status = WEXITSTATUS (status);

  return status;
}

// Collides with std::numeric_limits methods
#undef max

////////////////////////////////////////////////////////////////////////////////
unsigned burndown_size (unsigned ntasks)
{
  // Nearest 2
  if (ntasks < 20)
    return round_up_to (ntasks, 2);

  // Nearest 10
  if (ntasks < 50)
    return round_up_to (ntasks, 10);

  // Nearest 20
  if (ntasks < 100)
    return round_up_to (ntasks, 20);

  // Choose the number from here rounded up to the nearest 10% of the next
  // highest power of 10 or half of power of 10.
  const unsigned count = (unsigned) log10 (std::numeric_limits<unsigned>::max ());
  unsigned half = 500;
  unsigned full = 1000;

  // We start at two because we handle 5, 10, 50, and 100 above.
  for (unsigned i = 2; i < count; ++i)
  {
    if (ntasks < half)
      return round_up_to (ntasks, half / 10);

    if (ntasks < full)
      return round_up_to (ntasks, full / 10);

    half *= 10;
    full *= 10;
  }

  // Round up to max of unsigned.
  return std::numeric_limits<unsigned>::max ();
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   [  -> &open;
//   ]  -> &close;
const std::string encode (const std::string& value)
{
  std::string modified = value;

  str_replace (modified, "[",  "&open;");
  str_replace (modified, "]",  "&close;");

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   "  <- &dquot;
//   '  <- &squot; or &quot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
//   :  <- &colon;
const std::string decode (const std::string& value)
{
  if (value.find ('&') != std::string::npos)
  {
    std::string modified = value;

    // Supported encodings.
    str_replace (modified, "&open;",  "[");
    str_replace (modified, "&close;", "]");

    // Support for deprecated encodings.  These cannot be removed or old files
    // will not be parsable.  Not just old files - completed.data can contain
    // tasks formatted/encoded using these.
    str_replace (modified, "&dquot;", "\"");
    str_replace (modified, "&quot;",  "'");
    str_replace (modified, "&squot;", "'");  // Deprecated 2.0
    str_replace (modified, "&comma;", ",");  // Deprecated 2.0
    str_replace (modified, "&colon;", ":");  // Deprecated 2.0

    return modified;
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Escapes any unescaped character of type c within the given string
// e.g. ' ' -> '\ '
const std::string escape (const std::string& value, char c)
{
  std::string modified = value;
  char tmp[2] = {c, '\0'};
  std::string search  = tmp;
  std::string replace = "\\" + search;

  std::string::size_type pos = modified.find (search);
  while (pos != std::string::npos) {
    if ( modified[pos-1] != '\\' )
      modified.replace (pos, 1, replace);

    pos = modified.find (search, pos+1);
  }

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
// Accept a list of projects, and return an indented list
// that reflects the hierarchy.
//
//      Input  - "one"
//               "one.two"
//               "one.two.three"
//               "one.four"
//               "two"
//      Output - "one"
//               "  one.two"
//               "    one.two.three"
//               "  one.four"
//               "two"
//
// There are two optional arguments, 'whitespace', and 'delimiter',
//
//  - whitespace is the string used to build the prefixes of indented items.
//    - defaults to two spaces, "  "
//  - delimiter is the character used to split up projects into subprojects.
//    - defaults to the period, '.'
//
const std::vector<std::string> indentTree (
  const std::vector<std::string>& values,
  const std::string& whitespace /* = "  " */,
  char delimiter/* = '.' */)
{
  std::vector <std::string> modified;
  std::vector <std::string>::const_iterator i;
  for (i = values.begin (); i != values.end (); ++i)
    modified.push_back (indentProject (*i, whitespace, delimiter));

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
const std::string indentProject (
  const std::string& project,
  const std::string& whitespace /* = "  " */,
  char delimiter /* = '.' */)
{
  // Count the delimiters in *i.
  std::string prefix = "";
  std::string::size_type pos = 0;
  std::string::size_type lastpos = 0;
  while ((pos = project.find (delimiter, pos + 1)) != std::string::npos)
  {
    if (pos != project.size () - 1)
    {
      prefix += whitespace;
      lastpos = pos;
    }
  }

  std::string child = "";
  if (lastpos == 0)
    child = project;
  else
    child = project.substr (lastpos + 1);

  return prefix + child;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> extractParents (
  const std::string& project,
  const char& delimiter /* = '.' */)
{
  std::vector <std::string> vec;
  std::string::size_type pos = 0;
  std::string::size_type copyUntil = 0;
  while ((copyUntil = project.find (delimiter, pos + 1)) != std::string::npos)
  {
    if (copyUntil != project.size () - 1)
      vec.push_back (project.substr (0, copyUntil));
    pos = copyUntil;
  }
  return vec;
}

////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_TIMEGM
time_t timegm (struct tm *tm)
{
  time_t ret;
  char *tz;
  tz = getenv ("TZ");
  setenv ("TZ", "UTC", 1);
  tzset ();
  ret = mktime (tm);
  if (tz)
    setenv ("TZ", tz, 1);
  else
    unsetenv ("TZ");
  tzset ();
  return ret;
}
#endif

////////////////////////////////////////////////////////////////////////////////

