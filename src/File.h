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

#ifndef INCLUDED_FILE
#define INCLUDED_FILE

#include <stdio.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include "Path.h"

class File : public Path
{
public:
  File ();
  File (const Path&);
  File (const File&);
  File (const std::string&);
  virtual ~File ();

  File& operator= (const File&);

  virtual bool create ();
  virtual bool remove ();

  bool open ();
  bool openAndLock ();
  void close ();

  bool lock ();
  bool waitForLock ();

  void read (std::string&);
  void read (std::vector <std::string>&);

  void write (const std::string&);
  void write (const std::vector <std::string>&);

  void append (const std::string&);
  void append (const std::vector <std::string>&);

  virtual mode_t mode ();
  virtual size_t size () const;
  virtual time_t mtime () const;

  static bool create (const std::string&);
  static std::string read (const std::string&);
  static bool read (const std::string&, std::string&);
  static bool read (const std::string&, std::vector <std::string>&);
  static bool write (const std::string&, const std::string&);
  static bool write (const std::string&, const std::vector <std::string>&, bool addNewlines = true);
  static bool append (const std::string&, const std::string&);
  static bool append (const std::string&, const std::vector <std::string>&, bool addNewlines = true);
  static bool remove (const std::string&);

private:
  FILE* fh;
  int   h;
};

#endif
////////////////////////////////////////////////////////////////////////////////
