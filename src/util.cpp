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
// If <iostream> is included, put it after <stdio.h>, because it includes
// <stdio.h>, and therefore would ignore the _WITH_GETLINE.
#ifdef FREEBSD
#define _WITH_GETLINE
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
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
// Run a binary with args, capturing output.
int execute (
  const std::string& executable,
  const std::vector <std::string>& args,
  const std::string& input,
  std::string& output)
{
  pid_t pid;
  int pin[2], pout[2];
  fd_set rfds, wfds;
  struct timeval tv;
  int select_retval, read_retval, write_retval;
  char buf[16384];
  int written;
  const char * input_cstr = input.c_str ();

  signal(SIGPIPE, SIG_IGN); // Handled locally with EPIPE.

  if (pipe (pin) == -1)
    throw std::string (std::strerror (errno));
  if (pipe (pout) == -1)
    throw std::string (std::strerror (errno));

  if ((pid = fork ()) == -1)
    throw std::string (std::strerror (errno));

  if (pid == 0)
  {
    // This is only reached in the child
    if (dup2 (pin[0], STDIN_FILENO) == -1)
      throw std::string (std::strerror (errno));
    if (dup2 (pout[1], STDOUT_FILENO) == -1)
      throw std::string (std::strerror (errno));

    char** argv = new char* [args.size () + 1];
    for (unsigned int i = 0; i < args.size (); ++i)
      argv[i] = (char*) args[i].c_str ();

    argv[args.size ()] = NULL;
    _exit (execvp (executable.c_str (), argv));
  }

  // This is only reached in the parent
  close (pin[0]);   // Close the read end of the input pipe.
  close (pout[1]);  // Close the write end if the output pipe.

  read_retval = -1;
  written = 0;
  while (read_retval != 0 || input.size () - written != 0)
  {
    FD_ZERO (&rfds);
    if (read_retval != 0)
    {
      FD_SET (pout[0], &rfds);
    }

    FD_ZERO (&wfds);
    if (input.size () - written != 0)
    {
      FD_SET (pin[1], &wfds);
    }
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    select_retval = select(std::max(pout[0], pin[1]) + 1, &rfds, &wfds, NULL, &tv);

    if (select_retval == -1)
      throw std::string (std::strerror (errno));

    if (FD_ISSET (pin[1], &wfds))
    {
      write_retval = write(pin[1], input_cstr + written, input.size () - written);
      if (write_retval == -1)
      {
        if (errno == EPIPE)
        {
          // Child died (or closed the pipe) before reading all input.
          // We don't really care; pretend we wrote it all.
          write_retval = input.size () - written;
        }
        else
        {
          throw std::string (std::strerror (errno));
        }
      }
      written += write_retval;
    }

    if (FD_ISSET (pout[0], &rfds))
    {
      read_retval = read(pout[0], &buf, sizeof(buf)-1);
      if (read_retval == -1)
        throw std::string (std::strerror (errno));
      buf[read_retval] = '\0';
      output += buf;
    }
  }
  close (pin[1]);   // Close the write end of the input pipe.
  close (pout[0]);  // Close the read-end of the output pipe.

  int status = -1;
  if (wait (&status) == -1)
    throw std::string (std::strerror (errno));
  if (WIFEXITED (status))
  {
    status = WEXITSTATUS (status);
  }
  else
  {
    throw std::string ("Error: Could not get Hook exit status!");
  }

  return status;
}

// Collides with std::numeric_limits methods
#undef max

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

