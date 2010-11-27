////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_DIRECTORY
#define INCLUDED_DIRECTORY

#include "File.h"

class Directory : public File
{
public:
  Directory ();
  Directory (const Directory&);
  Directory (const File&);
  Directory (const Path&);
  Directory (const std::string&);
  virtual ~Directory ();

  Directory& operator= (const Directory&);

  virtual bool create ();
  virtual bool remove ();

  std::vector <std::string> list ();
  std::vector <std::string> listRecursive ();

private:
  void list (const std::string&, std::vector <std::string>&, bool);
};

#endif
////////////////////////////////////////////////////////////////////////////////
