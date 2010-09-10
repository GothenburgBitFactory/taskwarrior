////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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

#include <sys/file.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Global data.
static std::string gLogDir = ".";
static std::string gLogName = "default";
static bool gEnabled = true;
static const unsigned int maxMessageLength = 128;

static bool bDuplicates = false;
static std::string gPrior = "none";
static int gRepetitionCount = 0;

////////////////////////////////////////////////////////////////////////////////
void logSetDirectory (const std::string& dir)
{
  gLogDir = dir;
}

////////////////////////////////////////////////////////////////////////////////
void logSetName (const std::string& name)
{
  gLogName = name;
}

////////////////////////////////////////////////////////////////////////////////
void logEnable (const bool value)
{
  gEnabled = value;
}

////////////////////////////////////////////////////////////////////////////////
void logDuplicates (const bool value)
{
  bDuplicates = value;
}

////////////////////////////////////////////////////////////////////////////////
void getTimeStamp (std::string& ts)
{
  // Get time info.
  time_t now;
  time (&now);
  struct tm* t = localtime (&now);

  // Generate timestamp.
  char timestamp[20];
  sprintf (timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
                      t->tm_year + 1900,
                      t->tm_mon + 1,
                      t->tm_mday,
                      t->tm_hour,
                      t->tm_min,
                      t->tm_sec);

  ts = timestamp;
}

////////////////////////////////////////////////////////////////////////////////
// #Version: 1.0
// #Date: 17-Oct-2004
// #Fields: date time x-pid x-message
// 2004-10-18 00:12:00 12345 "Message"
void logWrite (const std::string& message)
{
  if (!gEnabled) return;

  if (!bDuplicates)
  {
    if (message == gPrior)
    {
      ++gRepetitionCount;
      return;
    }
    else
      gPrior = message;
  }

  // Get time info.
  time_t now;
  time (&now);
  struct tm* t = localtime (&now);

  // Sanitize 'message'.
  std::string sanitized = message;
  std::string::size_type bad;
  while ((bad = sanitized.find ("\"")) != std::string::npos)
    sanitized.replace (bad, 1, "'");

  if (sanitized.length () > maxMessageLength)
    sanitized = sanitized.substr (0, maxMessageLength) + "...";

  // Generate timestamp.
  char timestamp[20];
  sprintf (timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
                      t->tm_year + 1900,
                      t->tm_mon + 1,
                      t->tm_mday,
                      t->tm_hour,
                      t->tm_min,
                      t->tm_sec);

  // Determine file name.
  char file[_POSIX_PATH_MAX];
  sprintf (file, "%s/%s.%04d%02d%02d.log",
                 gLogDir.c_str (),
                 gLogName.c_str (),
                 t->tm_year + 1900,
                 t->tm_mon + 1,
                 t->tm_mday);

  // Determine whether file exists.
  bool exists = access (file, F_OK) != -1 ? true : false;

  // Create file if necessary, with header.
  FILE* log = fopen (file, "a");
  if (log)
  {
    // Lock file.
#if defined (__SVR4) && defined (__sun)
#else
    int fn = fileno (log);
    if (! flock (fn, LOCK_EX))
    {
#endif
      // Create the header, if the file did not exist.
      if (! exists)
      {
        fprintf (log, "# File:   %s.%04d%02d%02d.log\n",
                      gLogName.c_str (),
                      t->tm_year + 1900,
                      t->tm_mon + 1,
                      t->tm_mday);
        fprintf (log, "# Fields: date time x-pid x-message\n");
      }

      // Optionally write a repetition message.
      if (gRepetitionCount)
      {
        fprintf (log, "%s %d \"(Repeated %d times)\"\n",
                      timestamp,
                      (int) getpid (),
                      gRepetitionCount);
        gRepetitionCount = 0;
      }

      // Write entry.
      fprintf (log, "%s %d \"%s\"\n",
                    timestamp,
                    (int) getpid (),
                    sanitized.c_str ());
#if defined (__SVR4) && defined (__sun)
#else
    }
#endif

    // close file.
    fclose (log);

    if (! exists)
      chmod (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  }
}

////////////////////////////////////////////////////////////////////////////////
void logWrite (const char* message, ...)
{
  if (!gEnabled) return;

  // Crude and mostly ineffective check for buffer overrun.
  if (::strlen (message) >= 65536)
    throw std::string ("Data exceeds 65,536 bytes.  Break data into smaller chunks.");

  char buffer[65536];
  va_list args;
  va_start (args, message);
  vsnprintf (buffer, 65536, message, args);
  va_end (args);

  logWrite (std::string (buffer));
}

////////////////////////////////////////////////////////////////////////////////
